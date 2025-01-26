use clap::Parser;
use std::fs::File;
use std::io::{self, Read, Write};
use std::vec;

#[derive(Parser, Debug)]
struct Args {
    file_path: String,
    cave_size: i32,
}

fn read_file_to_vec(filename: &str) -> io::Result<Vec<u8>> {
    let mut file = File::open(filename)?;
    let mut buffer = Vec::new();
    file.read_to_end(&mut buffer)?;
    Ok(buffer)
}

fn insert_null_bytes(size: usize, buffer: Vec<u8>) -> Vec<u8> {
    let slice = vec![0; size];
    let mut buffer = buffer.clone();
    buffer.extend_from_slice(&slice);
    let mut new_buf = Vec::new();
    new_buf.extend_from_slice(&buffer);
    new_buf.extend_from_slice(&slice);
    new_buf
}

fn write_bytes_to_file(filename: &str, buffer: Vec<u8>) -> io::Result<()> {
    let mut file = File::create(filename)?;
    file.write_all(&buffer)?;
    Ok(())
}
fn main() {
    let args = Args::parse();

    //let filename = "C:\\Users\\sebas\\OneDrive\\Desktop\\Shakespere Top Ten Poems\\Sonnet 17.txt";
    let filename = args.file_path;
    let bytes = match read_file_to_vec(&filename) {
        Ok(bytes) => bytes,
        Err(e) => {
            eprintln!("Error: {e}");
            std::process::exit(-1);
        }
    };
    let size = args.cave_size;

    let new_buffer = insert_null_bytes(size as usize, bytes);

    match write_bytes_to_file(filename.as_str(), new_buffer) {
        Ok(()) => {
            println!("Successfully Wrote the bytes back with code cave");
        }
        Err(e) => {
            eprintln!("Error: {e}");
        }
    };
}

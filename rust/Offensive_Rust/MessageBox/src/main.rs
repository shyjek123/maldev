use windows_sys::{core::*, Win32::UI::WindowsAndMessaging::*};

fn main() {
    unsafe {
        MessageBoxW(0 as _, w!("Message"), w!("Title"), MB_OK);
    }
}

use screenshots::Screen;



fn main() {
    //let start = Instant::now();
    let screens = Screen::all().unwrap();

    for screen in screens {
        println!("hello {screen:?}");
        let image = screen.capture().unwrap();
        image
            .save("target.png")
            .unwrap();
    }
}

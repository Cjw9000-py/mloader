#include "mloader/loader.hxx"

using namespace mloader;



int main() {
    Path root = "data/";

    ConfigLoader loader { };
    loader.add_location(root);
    loader.scan();
    loader.dump();



}

/*

Goal code:

using namespace mloader;

int main() {
    BinaryDatabase database { "some/path/database.bin" };
    database
        .load()
        .activate();


    ImageAsset ape_texture = "textures/ape.png";
    AudioAsset background_sound = "music/nice_tunes.mp3";


    ... Other setup code, assets get loaded.

    Audio.play(background_sound.touch()); // touch returns asset and blocks until its loaded
    ape_texture.touch();

    while ... {
        ...
        Screen.show(ape_texture);
    }


 */
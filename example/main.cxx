#include <iostream>

#include "mloader/defs/registry.hxx"
#include "mloader/scanner.hxx"
#include "mloader/database/file.hxx"
#include "mtl/common.hxx"
#include "mtl/serial.hxx"

using namespace mtl::serial;

namespace {

struct House : mloader::Definition {
    str id;
    str address;
    int bedrooms = 0;
    bool has_garage = false;
    vec<str> occupants;

    VISIT() override {
        VIEW(id);
        VIEW(address);
        VIEW(bedrooms);
        VIEW(has_garage);
        VIEW_VEC(occupants);
    }

    use const str& identifier() cx override {
        return id;
    }
};

} // namespace

int main() {
    using namespace mloader;

    FilesystemDatabase database{mtl::fs::Path{"data"}};
    DatabaseScanner scanner(database);
    scanner.scan().dump();

    vec<mtl::fs::Path> config_files;
    config_files.reserve(scanner.configs().size());
    for (const auto& relative : scanner.configs()) {
        mtl::fs::Path absolute = database.root();
        const str rel = relative.as_posix();
        if (!rel.empty()) {
            absolute.with(rel);
        }
        config_files.emplace_back(std::move(absolute));
    }

    DefinitionRegistry registry;
    registry.register_type("House", [] {
        return make_uptr<House>();
    });
    registry.ingest(config_files);

    auto houses = registry.definitions("House");
    std::cout << "Loaded " << houses.size() << " house definitions\n";

    if (const auto* house = dynamic_cast<const House*>(registry.find("House", "starter_home"))) {
        std::cout << "Starter home at " << house->address << " has "
                  << house->bedrooms << " bedrooms and "
                  << (house->has_garage ? "a garage" : "no garage")
                  << ".\n";
    }

    return 0;
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

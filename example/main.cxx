#include "mloader/loader.hxx"



int main() {
    Path root = "data/";

    ConfigLoader loader { };
    loader.add_location(root);
    // loader.scan();

        auto rood = root / "ficl";

    printf("%s", rood.string().c_str());


}
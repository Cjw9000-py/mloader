#pragma once

#include "mtl.hxx"
#include "mtl/version.hxx"
#include "mtl/fs/path.hxx"

#include "yaml-cpp/yaml.h"


namespace mloader::extension {
    using YamlNode = YAML::Node;

    struct Extension {
        using PurePath = mtl::fs::PureUnixPath;

        str name;
        version version;
        vec<str> dependencies;
        vec<str> incompatible;
        vec<str> optional;
        PurePath definitions;
        struct {
            PurePath win32;
            PurePath linux_;
        } entrypoints;

        Extension from_config(YamlNode node);




    };

}
#include "mloader/extension/extension.hxx"

namespace mloader::extension {
    namespace {
        vec<str> read_string_sequence(const YamlNode& node) {
            vec<str> values;
            if (!node) {
                return values;
            }

            fassert(node.IsSequence(), "Extension config sequence must be a YAML sequence");
            values.reserve(static_cast<usize>(node.size()));
            for (const auto& entry : node) {
                fassert(entry.IsScalar(), "Extension config sequence entries must be scalars");
                values.emplace_back(entry.as<str>());
            }

            return values;
        }

        Extension::PurePath read_path_scalar(const YamlNode& node) {
            if (!node) {
                return {};
            }

            fassert(node.IsScalar(), "Extension config paths must be scalar values");
            return Extension::PurePath(node.as<str>());
        }
    } // namespace

    Extension Extension::from_config(YamlNode node) {
        fassert(node && node.IsMap(), "Extension config must be a YAML map");

        Extension out{};

        auto name_node = node["name"];
        fassert(name_node && name_node.IsScalar(), "Extension config requires a name");
        out.name = name_node.as<str>();

        auto version_node = node["version"];
        fassert(version_node && version_node.IsScalar(), "Extension config requires a version");
        const str version_text = version_node.as<str>();
        fassert(!version_text.empty(), "Extension version cannot be empty");
        out.version = version::from_string(version_text);

        out.dependencies = read_string_sequence(node["dependencies"]);
        out.incompatible = read_string_sequence(node["incompatible"]);
        out.optional = read_string_sequence(node["optional"]);

        out.definitions = read_path_scalar(node["definitions"]);

        if (auto entrypoints_node = node["entrypoints"]; entrypoints_node) {
            fassert(entrypoints_node.IsMap(), "Extension entrypoints must be a map");
            out.entrypoints.win32 = read_path_scalar(entrypoints_node["win32"]);
            out.entrypoints.linux_ = read_path_scalar(entrypoints_node["linux"]);
        }

        return out;
    }
}

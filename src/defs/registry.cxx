#include "mloader/defs/registry.hxx"

#include "mtl/common/string.hxx"
#include "mtl/error.hxx"

#include <yaml-cpp/yaml.h>

namespace mloader {

    void DefinitionRegistry::register_type(const str& type_name, Factory factory) {
        if (!factory) {
            throw RuntimeError("Attempted to register definition type '" + type_name + "' with null factory.");
        }

        auto [it, inserted] = m_factories.emplace(type_name, std::move(factory));
        if (!inserted) {
            throw RuntimeError("Definition type '" + type_name + "' is already registered.");
        }
    }

    void DefinitionRegistry::ingest(const mtl::fs::Path& file_path) {
        if (!file_path.exists()) {
            throw RuntimeError("Definition file not found: " + file_path.string());
        }

        auto contents = file_path.read_text();
        if (contents.empty()) {
            return;
        }

        YAML::Node root;
        try {
            root = YAML::Load(contents);
        } catch (const YAML::Exception& ex) {
            throw RuntimeError("Failed to parse YAML from '" + file_path.string() + "': " + ex.what());
        }

        if (!root || root.IsNull()) {
            return;
        }

        auto process_node = [&](const YAML::Node& node) {
            if (!node.IsMap()) {
                throw RuntimeError("Each definition entry in '" + file_path.string() + "' must be a mapping.");
            }

            auto type_node = node["type"];
            if (!type_node || !type_node.IsScalar()) {
                throw RuntimeError("Definition entry in '" + file_path.string() + "' is missing scalar 'type' field.");
            }

            str type_name;
            try {
                type_name = type_node.as<str>();
            } catch (const YAML::Exception& ex) {
                throw RuntimeError("Failed to convert definition type to string in '" + file_path.string() + "': " + ex.what());
            }

            ingest_node(type_name, node, file_path);
        };

        if (root.IsSequence()) {
            for (const auto& entry : root) {
                process_node(entry);
            }
        } else if (root.IsMap()) {
            process_node(root);
        } else {
            throw RuntimeError("Root of '" + file_path.string() + "' must be a mapping or sequence of mappings.");
        }
    }

    void DefinitionRegistry::ingest(const vec<mtl::fs::Path>& files) {
        for (const auto& file : files) {
            ingest(file);
        }
    }

    vec<str> DefinitionRegistry::types() const {
        vec<str> names;
        names.reserve(m_definitions.size());
        for (const auto& [type_name, _] : m_definitions) {
            names.emplace_back(type_name);
        }
        return names;
    }

    vec<const Definition*> DefinitionRegistry::definitions(const str& type_name) const {
        vec<const Definition*> listed;
        auto it = m_definitions.find(type_name);
        if (it == m_definitions.end()) {
            return listed;
        }

        listed.reserve(it->second.size());
        for (const auto& [_, definition] : it->second) {
            listed.emplace_back(definition.get());
        }
        return listed;
    }

    const Definition* DefinitionRegistry::find(const str& type_name, const str& identifier) const {
        auto type_it = m_definitions.find(type_name);
        if (type_it == m_definitions.end()) {
            return nullptr;
        }

        const auto& bucket = type_it->second;
        auto def_it = bucket.find(identifier);
        if (def_it == bucket.end()) {
            return nullptr;
        }

        return def_it->second.get();
    }

    void DefinitionRegistry::clear() {
        m_definitions.clear();
    }

    void DefinitionRegistry::ingest_node(const str& type_name, const YAML::Node& node, const mtl::fs::Path& source) {
        auto factory_it = m_factories.find(type_name);
        if (factory_it == m_factories.end()) {
            throw RuntimeError("No factory registered for definition type '" + type_name + "' (found in " + source.string() + ").");
        }

        auto definition = factory_it->second();
        if (!definition) {
            throw RuntimeError("Factory for definition type '" + type_name + "' returned null (source: " + source.string() + ").");
        }

        try {
            definition->load_yaml(node);
        } catch (const YAML::Exception& ex) {
            throw RuntimeError("Failed to load definition of type '" + type_name + "' from " + source.string() + ": " + ex.what());
        }

        const str& id = definition->identifier();
        if (id.empty()) {
            throw RuntimeError("Definition of type '" + type_name + "' in " + source.string() + " produced an empty identifier.");
        }

        auto& bucket = m_definitions[type_name];
        if (bucket.contains(id)) {
            throw RuntimeError("Duplicate definition '" + id + "' for type '" + type_name + "' encountered in " + source.string() + ".");
        }

        bucket.emplace(id, std::move(definition));
    }

} // namespace mloader

#pragma once

#include "mtl/common.hxx"
#include "mtl/fs/path/path.hxx"

#include "mloader/defs/definition.hxx"

namespace YAML {
    class Node;
}

namespace mloader {

    struct DefinitionRegistry {
        using DefinitionPtr = uptr<Definition>;
        using Factory = function<DefinitionPtr()>;

        void register_type(const str& type_name, Factory factory);

        void ingest(const mtl::fs::Path& file_path);
        void ingest(const vec<mtl::fs::Path>& files);

        use vec<str> types() const;
        use vec<const Definition*> definitions(const str& type_name) const;
        use const Definition* find(const str& type_name, const str& identifier) const;

        void clear();

    protected:
        void ingest_node(const str& type_name, const YAML::Node& node, const mtl::fs::Path& source);

        umap<str, Factory> m_factories;
        umap<str, umap<str, DefinitionPtr>> m_definitions;
    };

} // namespace mloader

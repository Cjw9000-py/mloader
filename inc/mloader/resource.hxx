#pragma once
#include "mtl/common.hxx"


namespace mloader {

    struct Resource {
        ctor Resource() = default;



        Resource& touch();
        const Resource& touch() const;

    };




}

#pragma once


#include "common.hxx"


namespace mloader {
    struct Resource;

    struct ResourceDatabase {

        ///// Constructors

        ctor ResourceDatabase() = default;
        virtual ~ResourceDatabase() = default;


        ///// Loading methods

        /**
         * Checks if the database has finished loading and is in a usable state.
         */
        prop virtual bool is_loaded() cx = 0;

        /**
         * Initializes and loads the database into memory.
         * It may be a no-op for some implementations.
         */
        virtual void load() const = 0;

        /**
         * Unloads the database and releases associated resources.
         */
        virtual void unload() const = 0;

        /**
         * Ensures a resource is in a ready state.
         *
         * This finalizes the resource lifecycle by loading its raw data
         * (if not already done) and parsing it. Assumes the resource was
         * registered using `resolve()` and throws if it's unknown.
         */
        virtual void request(Resource* resource) = 0;

        /**
         * Fully releases a resource, unmapping its memory and deleting it.
         * Should only be called on resources created and tracked by `resolve()`.
         */
        virtual void release(Resource* resource) = 0;


        void




    };
}
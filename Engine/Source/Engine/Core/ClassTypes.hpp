#pragma once

// Prohibits instances of the given class.
#define ENG_STATIC_CLASS(name) \
    name() = delete; \
    name(name const&) = delete; \
    name(name&&) = delete; \
    name& operator=(name const&) = delete; \
    name& operator=(name&&) = delete; \
    ~name() = delete

// Prohibits instances of the given class from being moved, but still can be copied, constructed, and destructed.
#define ENG_IMMOVABLE_CLASS(name) \
    name(name&&) = delete; \
    name& operator=(name&&) = delete

// Prohibits instances of the given class from being copied, but still can be moved, constructed, and destructed.
#define ENG_UNCOPYABLE_CLASS(name) \
    name(name const&) = delete; \
    name& operator=(name const&) = delete

// Prohibits instances of the given class from being copied or moved, but still can be constructed and destructed.
#define ENG_IMMOVABLE_UNCOPYABLE_CLASS(name) \
    ENG_IMMOVABLE_CLASS(name); \
    ENG_UNCOPYABLE_CLASS(name)

// Prohibits instances of the given class from being copied or moved, but still can be constructed and destructed.
// Also provides a default no-arg constructor.
#define ENG_IMMOVABLE_UNCOPYABLE_DEFAULTABLE_CLASS(name) \
    ENG_IMMOVABLE_UNCOPYABLE_CLASS(name); \
    name() noexcept = default

// Prohibits instances of the given class from being copied or moved, but still can be constructed and destructed.
// Also provides a default no-arg constructor and a virtual default destructor.
#define ENG_IMMOVABLE_UNCOPYABLE_INHERITABLE_CLASS(name) \
    ENG_IMMOVABLE_UNCOPYABLE_DEFAULTABLE_CLASS(name); \
    virtual ~name() noexcept = default

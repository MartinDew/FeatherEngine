target("simplemath")
    set_kind("static")
    set_warnings("none")
    add_files("SimpleMath.cpp")
    add_headerfiles("SimpleMath.h", "SimpleMath.inl")
    -- Public: consumers of simplemath get the include dir and these defines
    add_includedirs("$(scriptdir)", {public = true})
    add_defines("WIN32_LEAN_AND_MEAN", "NOMINMAX", {public = true})
    -- DirectXMath types flow through SimpleMath's public headers
    add_packages("directxmath", {public = true})
target_end()

function(add_calc_plugin target_name source_file)
    add_library(${target_name} SHARED "${source_file}")

    target_include_directories(${target_name} PRIVATE
            "${CMAKE_SOURCE_DIR}/include"
    )

    set_target_properties(${target_name} PROPERTIES
            PREFIX ""
            LIBRARY_OUTPUT_DIRECTORY "${PLUGINS_OUTPUT_DIR}"
            RUNTIME_OUTPUT_DIRECTORY "${PLUGINS_OUTPUT_DIR}"
            ARCHIVE_OUTPUT_DIRECTORY "${PLUGINS_OUTPUT_DIR}"
    )

    foreach (cfg IN LISTS CMAKE_CONFIGURATION_TYPES)
        string(TOUPPER "${cfg}" cfg_upper)
        set_target_properties(${target_name} PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY_${cfg_upper} "${PLUGINS_OUTPUT_DIR}/${cfg}"
                LIBRARY_OUTPUT_DIRECTORY_${cfg_upper} "${PLUGINS_OUTPUT_DIR}/${cfg}"
                ARCHIVE_OUTPUT_DIRECTORY_${cfg_upper} "${PLUGINS_OUTPUT_DIR}/${cfg}"
        )
    endforeach ()
endfunction()

add_calc_plugin(funcsin "${CMAKE_CURRENT_SOURCE_DIR}/sin/sin_plugin.cpp")
add_calc_plugin(funcdeg "${CMAKE_CURRENT_SOURCE_DIR}/deg/deg_plugin.cpp")
add_calc_plugin(funcln "${CMAKE_CURRENT_SOURCE_DIR}/ln/ln_plugin.cpp")
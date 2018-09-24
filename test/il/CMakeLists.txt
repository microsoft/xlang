cmake_minimum_required(VERSION 3.9)

set(cpp_exe ${CMAKE_CURRENT_BINARY_DIR}/../../tool/cpp/cpp.exe)

get_filename_component(output_dir ${CMAKE_CURRENT_SOURCE_DIR}/../output REALPATH)

set(foundation_il ${CMAKE_CURRENT_SOURCE_DIR}/foundation.il)
set(simple_il ${CMAKE_CURRENT_SOURCE_DIR}/simple.il)
set(component_il ${CMAKE_CURRENT_SOURCE_DIR}/component.il)

set(windows_foundation_h ${output_dir}/foundation/winrt/Windows.Foundation.h)
set(simple_h ${output_dir}/simple/winrt/simple.h)
set(component_h ${output_dir}/component/source/winrt/Component.h)

set(foundation_winmd ${output_dir}/foundation.winmd)
set(simple_winmd ${output_dir}/simple.winmd)
set(component_winmd ${output_dir}/component.winmd)

set(projection_headers ${windows_foundation_h} ${simple_h} ${component_h})

add_custom_command(OUTPUT ${output_dir}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${output_dir}
)

add_custom_command(OUTPUT ${foundation_winmd}
  COMMAND ilasm ${foundation_il} -MDV=\"WindowsRuntime 1.2\" -dll -output=${foundation_winmd}
  DEPENDS ${foundation_il} ${output_dir}
)

add_custom_command(OUTPUT ${simple_winmd}
  COMMAND ilasm ${simple_il} -MDV=\"WindowsRuntime 1.2\" -dll -output=${simple_winmd}
  DEPENDS ${simple_il} ${output_dir}
)

add_custom_command(OUTPUT ${component_winmd}
  COMMAND ilasm ${component_il} -MDV=\"WindowsRuntime 1.2\" -dll -output=${component_winmd}
  DEPENDS ${component_il} ${output_dir}
)

add_custom_command(OUTPUT ${windows_foundation_h} 
  COMMAND ${cpp_exe} -in ${foundation_winmd} -output ${output_dir}/foundation -verbose
  DEPENDS ${foundation_winmd}
)

add_custom_command(OUTPUT ${simple_h}
  COMMAND ${cpp_exe} -in ${simple_winmd} -ref ${foundation_winmd} -output ${output_dir}/simple -verbose
  DEPENDS ${simple_winmd} ${foundation_winmd}
)

add_custom_command(OUTPUT ${component_h} 
  COMMAND ${cpp_exe} -in ${component_winmd} -ref ${foundation_winmd} -output ${output_dir}/component/source -component ${output_dir}/component/gen -verbose
  DEPENDS ${component_winmd} ${foundation_winmd}
)
  
add_custom_target(test_il ALL DEPENDS ${projection_headers})
add_dependencies(test_il cpp)

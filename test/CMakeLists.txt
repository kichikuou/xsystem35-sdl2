configure_file(testSA.ALD testSA.ALD COPYONLY)
configure_file(System39.ain System39.ain COPYONLY)

add_test(
  NAME scenario_tests
  COMMAND xsystem35
    -M0
    -renderer software
    -texthook print
    -ttfont_gothic ${PROJECT_SOURCE_DIR}/fonts/MTLc3m.ttf
    -ttfont_mincho ${PROJECT_SOURCE_DIR}/fonts/mincho.ttf
  )
set_tests_properties(scenario_tests PROPERTIES
  ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy"
  PASS_REGULAR_EXPRESSION "0 tests failed"
  TIMEOUT 5
  )

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(googletest)

add_subdirectory(unit)

add_subdirectory(checker)
add_subdirectory(validator)
add_subdirectory(generator)
add_subdirectory(interactor)

# this is required
SET(CMAKE_SYSTEM_NAME Linux)

# specify the cross compiler
SET(CMAKE_C_COMPILER   $ENV{CTC_ATOM_HOME}/bin/i686-aldebaran-linux-gnu-gcc)
SET(CMAKE_CXX_COMPILER $ENV{CTC_ATOM_HOME}/bin/i686-aldebaran-linux-gnu-g++)

# where is the target environment
SET(CMAKE_FIND_ROOT_PATH
        $ENV{CTC_ATOM_HOME}/i686-aldebaran-linux-gnu/sysroot
        $ENV{CTC_ATOM_HOME}/i686-aldebaran-linux-gnu
        $ENV{THIRDPARTY_HOME}
)

# search for programs in the build host directories (not necessary)
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# configure Boost and others
SET(BOOST_ROOT $ENV{CTC_ATOM_HOME}/boost)

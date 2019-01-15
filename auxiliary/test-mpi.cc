#include <cstring>
#include <mpi.h>
#include <vector>

// Boost command-line options
#if defined(ENABLE_BOOST)
  #include <boost/program_options.hpp>
  using namespace boost::program_options;
#endif

#include <gtest/gtest.h>
#include "cinchtest.h"

//----------------------------------------------------------------------------//
// Allow extra initialization steps to be added by the user.
//----------------------------------------------------------------------------//

#if defined(CINCH_OVERRIDE_DEFAULT_INITIALIZATION_DRIVER)
  int driver_initialization(int argc, char ** argv);
#else
  inline int driver_initialization(int argc, char ** argv) { return 0; }
#endif

//----------------------------------------------------------------------------//
// Main
//----------------------------------------------------------------------------//

int main(int argc, char ** argv) {

  // Initialize the MPI runtime
  MPI_Init(&argc, &argv);
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // Initialize the GTest runtime
  ::testing::InitGoogleTest(&argc, argv);

  // Initialize tags to output all tag groups from CLOG
  std::string tags("all");

#if defined(ENABLE_BOOST)
  options_description desc("Cinch test options");  

  // Add command-line options
  desc.add_options()
    ("help,h", "Print this message and exit.")
    ("tags,t", value(&tags)->implicit_value("0"),
      "Enable the specified output tags, e.g., --tags=tag1,tag2."
      " Passing --tags by itself will print the available tags.")
    ;
  variables_map vm;
  parsed_options parsed =
    command_line_parser(argc, argv).options(desc).allow_unregistered().run();
  store(parsed, vm);

  notify(vm);

  if(vm.count("help")) {
    if(rank == 0) {
      std::cout << desc << std::endl;
    } // if

    MPI_Finalize();

    return 1;
  } // if
#endif // ENABLE_BOOST

  int result(0);

  if(tags == "0") {
    // Output the available tags
    if(rank == 0) {
      std::cout << "Available tags (CLOG):" << std::endl;

      for(auto t: clog_tag_map()) {
        std::cout << "  " << t.first << std::endl;
      } // for
    } // if
  }
  else {
    // Initialize the cinchlog runtime
    clog_init(tags);

    // Call the user-provided initialization function
    driver_initialization(argc, argv);

    // Run the tests for this target.
    result = RUN_ALL_TESTS();
  } // if

  // Shutdown the MPI runtime
  MPI_Finalize();

  return result;
} // main

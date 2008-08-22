// This file is part of Eigen, a lightweight C++ template library
// for linear algebra. Eigen itself is part of the KDE project.
//
// Copyright (C) 2006-2008 Benoit Jacob <jacob@math.jussieu.fr>
// Copyright (C) 2008 Gael Guennebaud <g.gael@free.fr>
//
// Eigen is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or (at your option) any later version.
//
// Alternatively, you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// Eigen is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License or the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License and a copy of the GNU General Public License along with
// Eigen. If not, see <http://www.gnu.org/licenses/>.

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include <vector>

#ifndef EIGEN_TEST_FUNC
#error EIGEN_TEST_FUNC must be defined
#endif

#define DEFAULT_REPEAT 10

namespace Eigen
{
  static std::vector<std::string> g_test_stack;
  static int g_repeat;
}

#define EI_PP_MAKE_STRING2(S) #S
#define EI_PP_MAKE_STRING(S) EI_PP_MAKE_STRING2(S)

#define EI_PP_CAT2(a,b) a ## b
#define EI_PP_CAT(a,b) EI_PP_CAT2(a,b)

#ifndef EIGEN_NO_ASSERTION_CHECKING

  namespace Eigen
  {
    static const bool should_raise_an_assert = false;

    // Used to avoid to raise two exceptions at a time in which
    // case the exception is not properly catched.
    // This may happen when a second exceptions is raise in a destructor.
    static bool no_more_assert = false;

    struct ei_assert_exception
    {
      ei_assert_exception(void) {}
      ~ei_assert_exception() { Eigen::no_more_assert = false; }
    };
  }

  // If EIGEN_DEBUG_ASSERTS is defined and if no assertion is raised while
  // one should have been, then the list of excecuted assertions is printed out.
  //
  // EIGEN_DEBUG_ASSERTS is not enabled by default as it
  // significantly increases the compilation time
  // and might even introduce side effects that would hide
  // some memory errors.
  #ifdef EIGEN_DEBUG_ASSERTS

    namespace Eigen
    {
      static bool ei_push_assert = false;
      static std::vector<std::string> ei_assert_list;
    }

    #define ei_assert(a)                       \
      if( (!(a)) && (!no_more_assert) )     \
      {                                     \
        Eigen::no_more_assert = true;       \
        throw Eigen::ei_assert_exception(); \
      }                                     \
      else if (Eigen::ei_push_assert)       \
      {                                     \
        ei_assert_list.push_back(std::string(EI_PP_MAKE_STRING(__FILE__)" ("EI_PP_MAKE_STRING(__LINE__)") : "#a) ); \
      }

    #define VERIFY_RAISES_ASSERT(a)                                               \
      {                                                                           \
        Eigen::no_more_assert = false;                                            \
        try {                                                                     \
          Eigen::ei_assert_list.clear();                                          \
          Eigen::ei_push_assert = true;                                           \
          a;                                                                      \
          Eigen::ei_push_assert = false;                                          \
          std::cerr << "One of the following asserts should have been raised:\n"; \
          for (uint ai=0 ; ai<ei_assert_list.size() ; ++ai)                       \
            std::cerr << "  " << ei_assert_list[ai] << "\n";                      \
          VERIFY(Eigen::should_raise_an_assert && # a);                           \
        } catch (Eigen::ei_assert_exception e) {                                  \
          Eigen::ei_push_assert = false; VERIFY(true);                            \
        }                                                                         \
      }

  #else // EIGEN_DEBUG_ASSERTS

    #define ei_assert(a) \
      if( (!(a)) && (!no_more_assert) )     \
      {                                     \
        Eigen::no_more_assert = true;       \
        throw Eigen::ei_assert_exception(); \
      }

    #define VERIFY_RAISES_ASSERT(a) {                             \
        Eigen::no_more_assert = false;                            \
        try { a; VERIFY(Eigen::should_raise_an_assert && # a); }  \
        catch (Eigen::ei_assert_exception e) { VERIFY(true); }    \
      }

  #endif // EIGEN_DEBUG_ASSERTS

  #define EIGEN_USE_CUSTOM_ASSERT

#else // EIGEN_NO_ASSERTION_CHECKING

  #define VERIFY_RAISES_ASSERT(a) {}

#endif // EIGEN_NO_ASSERTION_CHECKING


#define EIGEN_INTERNAL_DEBUGGING
#include <Eigen/Core>

namespace Eigen {
#include <Eigen/src/Array/Random.h>
}

#define VERIFY(a) do { if (!(a)) { \
    std::cerr << "Test " << g_test_stack.back() << " failed in "EI_PP_MAKE_STRING(__FILE__) << " (" << EI_PP_MAKE_STRING(__LINE__) << ")" \
      << std::endl << "    " << EI_PP_MAKE_STRING(a) << std::endl << std::endl; \
    exit(2); \
  } } while (0)

#define VERIFY_IS_APPROX(a, b) VERIFY(test_ei_isApprox(a, b))
#define VERIFY_IS_NOT_APPROX(a, b) VERIFY(!test_ei_isApprox(a, b))
#define VERIFY_IS_MUCH_SMALLER_THAN(a, b) VERIFY(test_ei_isMuchSmallerThan(a, b))
#define VERIFY_IS_NOT_MUCH_SMALLER_THAN(a, b) VERIFY(!test_ei_isMuchSmallerThan(a, b))
#define VERIFY_IS_APPROX_OR_LESS_THAN(a, b) VERIFY(test_ei_isApproxOrLessThan(a, b))
#define VERIFY_IS_NOT_APPROX_OR_LESS_THAN(a, b) VERIFY(!test_ei_isApproxOrLessThan(a, b))

#define CALL_SUBTEST(FUNC) do { \
    g_test_stack.push_back(EI_PP_MAKE_STRING(FUNC)); \
    FUNC; \
    g_test_stack.pop_back(); \
  } while (0)

namespace Eigen {

template<typename T> inline typename NumTraits<T>::Real test_precision();
template<> inline int test_precision<int>() { return 0; }
template<> inline float test_precision<float>() { return 1e-4f; }
template<> inline double test_precision<double>() { return 1e-6; }
template<> inline float test_precision<std::complex<float> >() { return test_precision<float>(); }
template<> inline double test_precision<std::complex<double> >() { return test_precision<double>(); }

inline bool test_ei_isApprox(const int& a, const int& b)
{ return ei_isApprox(a, b, test_precision<int>()); }
inline bool test_ei_isMuchSmallerThan(const int& a, const int& b)
{ return ei_isMuchSmallerThan(a, b, test_precision<int>()); }
inline bool test_ei_isApproxOrLessThan(const int& a, const int& b)
{ return ei_isApproxOrLessThan(a, b, test_precision<int>()); }

inline bool test_ei_isApprox(const float& a, const float& b)
{ return ei_isApprox(a, b, test_precision<float>()); }
inline bool test_ei_isMuchSmallerThan(const float& a, const float& b)
{ return ei_isMuchSmallerThan(a, b, test_precision<float>()); }
inline bool test_ei_isApproxOrLessThan(const float& a, const float& b)
{ return ei_isApproxOrLessThan(a, b, test_precision<float>()); }

inline bool test_ei_isApprox(const double& a, const double& b)
{ return ei_isApprox(a, b, test_precision<double>()); }
inline bool test_ei_isMuchSmallerThan(const double& a, const double& b)
{ return ei_isMuchSmallerThan(a, b, test_precision<double>()); }
inline bool test_ei_isApproxOrLessThan(const double& a, const double& b)
{ return ei_isApproxOrLessThan(a, b, test_precision<double>()); }

inline bool test_ei_isApprox(const std::complex<float>& a, const std::complex<float>& b)
{ return ei_isApprox(a, b, test_precision<std::complex<float> >()); }
inline bool test_ei_isMuchSmallerThan(const std::complex<float>& a, const std::complex<float>& b)
{ return ei_isMuchSmallerThan(a, b, test_precision<std::complex<float> >()); }

inline bool test_ei_isApprox(const std::complex<double>& a, const std::complex<double>& b)
{ return ei_isApprox(a, b, test_precision<std::complex<double> >()); }
inline bool test_ei_isMuchSmallerThan(const std::complex<double>& a, const std::complex<double>& b)
{ return ei_isMuchSmallerThan(a, b, test_precision<std::complex<double> >()); }

template<typename Derived1, typename Derived2>
inline bool test_ei_isApprox(const MatrixBase<Derived1>& m1,
                   const MatrixBase<Derived2>& m2)
{
  return m1.isApprox(m2, test_precision<typename ei_traits<Derived1>::Scalar>());
}

template<typename Derived1, typename Derived2>
inline bool test_ei_isMuchSmallerThan(const MatrixBase<Derived1>& m1,
                                   const MatrixBase<Derived2>& m2)
{
  return m1.isMuchSmallerThan(m2, test_precision<typename ei_traits<Derived1>::Scalar>());
}

template<typename Derived>
inline bool test_ei_isMuchSmallerThan(const MatrixBase<Derived>& m,
                                   const typename NumTraits<typename ei_traits<Derived>::Scalar>::Real& s)
{
  return m.isMuchSmallerThan(s, test_precision<typename ei_traits<Derived>::Scalar>());
}

template<typename T> T test_random();

template<> int test_random() { return ei_random<int>(-100,100); }
template<> float test_random() { return float(ei_random<int>(-1000,1000)) / 256.f; }
template<> double test_random() { return double(ei_random<int>(-1000,1000)) / 256.; }
template<> std::complex<float> test_random()
{ return std::complex<float>(test_random<float>(),test_random<float>()); }
template<> std::complex<double> test_random()
{ return std::complex<double>(test_random<double>(),test_random<double>()); }

template<typename MatrixType>
MatrixType test_random_matrix(int rows = MatrixType::RowsAtCompileTime, int cols = MatrixType::ColsAtCompileTime)
{
  MatrixType res(rows, cols);
  for (int j=0; j<cols; ++j)
    for (int i=0; i<rows; ++i)
      res.coeffRef(i,j) = test_random<typename MatrixType::Scalar>();
  return res;
}

} // end namespace Eigen


// forward declaration of the main test function
void EI_PP_CAT(test_,EIGEN_TEST_FUNC)();

using namespace Eigen;
using namespace std;

int main(int argc, char *argv[])
{
    bool has_set_repeat = false;
    bool has_set_seed = false;
    bool need_help = false;
    unsigned int seed = 0;
    int repeat = DEFAULT_REPEAT;

    for(int i = 1; i < argc; i++)
    {
      if(argv[i][0] == 'r')
      {
        if(has_set_repeat)
        {
          cout << "Argument " << argv[i] << " conflicting with a former argument" << endl;
          return 1;
        }
        repeat = atoi(argv[i]+1);
        has_set_repeat = true;
        if(repeat <= 0)
        {
          cout << "Invalid \'repeat\' value " << argv[i]+1 << endl;
          return 1;
        }
      }
      else if(argv[i][0] == 's')
      {
        if(has_set_seed)
        {
          cout << "Argument " << argv[i] << " conflicting with a former argument" << endl;
          return 1;
        }
        seed = strtoul(argv[i]+1, 0, 10);
        has_set_seed = true;
        bool ok = seed!=0;
        if(!ok)
        {
          cout << "Invalid \'seed\' value " << argv[i]+1 << endl;
          return 1;
        }
      }
      else
      {
        need_help = true;
      }
    }

    if(need_help)
    {
      cout << "This test application takes the following optional arguments:" << endl;
      cout << "  rN     Repeat each test N times (default: " << DEFAULT_REPEAT << ")" << endl;
      cout << "  sN     Use N as seed for random numbers (default: based on current time)" << endl;
      return 1;
    }

    if(!has_set_seed) seed = (unsigned int) time(NULL);
    if(!has_set_repeat) repeat = DEFAULT_REPEAT;

    cout << "Initializing random number generator with seed " << seed << endl;
    srand(seed);
    cout << "Repeating each test " << repeat << " times" << endl;

    Eigen::g_repeat = repeat;
    Eigen::g_test_stack.push_back(EI_PP_MAKE_STRING(EIGEN_TEST_FUNC));

    EI_PP_CAT(test_,EIGEN_TEST_FUNC)();
    return 0;
}




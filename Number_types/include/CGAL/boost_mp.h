// Copyright (c) 2017
// INRIA Saclay-Ile de France (France),
//
// This file is part of CGAL (www.cgal.org); you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 3 of the License,
// or (at your option) any later version.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
//
// Author(s)     : Marc Glisse

#ifndef CGAL_BOOST_MP_H
#define CGAL_BOOST_MP_H

#if BOOST_VERSION >= 105300

#include <CGAL/number_type_basic.h>
#include <CGAL/Modular_traits.h>
// We can't just include all Boost.Multiprecision here...
#include <boost/multiprecision/number.hpp>

// TODO: work on the coercions (end of the file)

namespace CGAL {

// Algebraic_structure_traits

template <class T, class = boost::mpl::int_<boost::multiprecision::number_category<T>::value> >
struct AST_boost_mp;

template <class NT>
struct AST_boost_mp <NT, boost::mpl::int_<boost::multiprecision::number_kind_integer> >
  : Algebraic_structure_traits_base< NT, Euclidean_ring_tag > {
    typedef NT			Type;
    typedef Euclidean_ring_tag	Algebraic_category;
    typedef Boolean_tag<std::numeric_limits<Type>::is_exact>	Is_exact;
    typedef Tag_false		Is_numerical_sensitive;

    struct Is_zero: public std::unary_function<Type ,bool> {
        bool operator()( const Type& x) const {
            return x.is_zero();
        }
    };

    struct Div:
        public std::binary_function<Type ,Type, Type> {
        template <typename T, typename U>
        Type operator()(const T& x, const U& y) const {
            return x / y;
        }
    };

    struct Mod:
        public std::binary_function<Type ,Type, Type> {
        template <typename T, typename U>
        Type operator()(const T& x, const U& y) const {
            return x % y;
        }
    };

    struct Gcd : public std::binary_function<Type, Type, Type> {
        template <typename T, typename U>
        Type operator()(const T& x, const U& y) const {
            return boost::multiprecision::gcd(x, y);
        }
    };

    struct Sqrt : public std::unary_function<Type, Type> {
        template <typename T>
        Type operator()(const T& x) const {
            return boost::multiprecision::sqrt(x);
        }
    };
};

template <class NT>
struct AST_boost_mp <NT, boost::mpl::int_<boost::multiprecision::number_kind_rational> >
  : public Algebraic_structure_traits_base< NT , Field_tag >  {
  public:
    typedef NT			Type;
    typedef Field_tag		Algebraic_category;
    typedef Tag_true		Is_exact;
    typedef Tag_false		Is_numerical_sensitive;

    struct Is_zero: public std::unary_function<Type ,bool> {
        bool operator()( const Type& x) const {
            return x.is_zero();
        }
    };

    struct Div:
        public std::binary_function<Type ,Type, Type> {
        template <typename T, typename U>
        Type operator()(const T& x, const U& y) const {
            return x / y;
        }
    };
};

// Real_embeddable_traits

template <class NT>
struct RET_boost_mp_base
    : public INTERN_RET::Real_embeddable_traits_base< NT , CGAL::Tag_true > {

    typedef NT Type;

    struct Is_zero: public std::unary_function<Type ,bool> {
        bool operator()( const Type& x) const {
            return x.is_zero();
        }
    };

    struct Is_positive: public std::unary_function<Type ,bool> {
        bool operator()( const Type& x) const {
            return x.sign() > 0;
        }
    };

    struct Is_negative: public std::unary_function<Type ,bool> {
        bool operator()( const Type& x) const {
            return x.sign() < 0;
        }
    };

    struct Abs : public std::unary_function<Type, Type> {
        template <typename T>
        Type operator()(const T& x) const {
            return boost::multiprecision::abs(x);
        }
    };

    struct Sgn : public std::unary_function<Type, ::CGAL::Sign> {
        ::CGAL::Sign operator()(Type const& x) const {
            return CGAL::sign(x.sign());
        }
    };

    struct Compare
        : public std::binary_function<Type, Type, Comparison_result> {
        Comparison_result operator()(const Type& x, const Type& y) const {
            return CGAL::sign(x.compare(y));
        }
    };

    struct To_double
        : public std::unary_function<Type, double> {
        double operator()(const Type& x) const {
            return x.template convert_to<double>();
        }
    };

    struct To_interval
        : public std::unary_function< Type, std::pair< double, double > > {
        std::pair<double, double>
        operator()(const Type& x) const {
	    // assume the conversion is within 1 ulp
	    // adding IA::smallest() doesn't work because inf-e=inf, even rounded down.
	    double d = x.template convert_to<double>();
	    double inf = std::numeric_limits<double>::infinity();
	    return std::pair<double, double> (nextafter (d, -inf), nextafter (d, inf));
        }
    };
};

template <class T, class = boost::mpl::int_<boost::multiprecision::number_category<T>::value> >
struct RET_boost_mp;

template <class NT>
struct RET_boost_mp <NT, boost::mpl::int_<boost::multiprecision::number_kind_integer> >
    : RET_boost_mp_base <NT> {};

template <class NT>
struct RET_boost_mp <NT, boost::mpl::int_<boost::multiprecision::number_kind_rational> >
    : RET_boost_mp_base <NT> {
#if BOOST_VERSION < 105700
    typedef NT Type;
    struct To_interval
        : public std::unary_function< Type, std::pair< double, double > > {
        std::pair<double, double>
        operator()(const Type& x) const {
	    std::pair<double,double> p_num = CGAL::to_interval (numerator (x));
	    std::pair<double,double> p_den = CGAL::to_interval (denominator (x));
	    typedef Interval_nt<false> IA;
	    IA::Protector P;
	    // assume the conversion is within 1 ulp for integers, but the conversion from rational may be unsafe, see boost trac #10085
	    IA i_num (p_num.first, p_num.second);
	    IA i_den (p_den.first, p_den.second);
	    IA i = i_num / i_den;
            return std::pair<double, double>(i.inf(), i.sup());
        }
    };
#endif
};

// Modular_traits

template <class T, class = boost::mpl::int_<boost::multiprecision::number_category<T>::value> >
struct MT_boost_mp {
  typedef T NT;
  typedef ::CGAL::Tag_false Is_modularizable;
  typedef ::CGAL::Null_functor Residue_type;
  typedef ::CGAL::Null_functor Modular_image;
  typedef ::CGAL::Null_functor Modular_image_representative;
};

template <class T>
struct MT_boost_mp <T, boost::mpl::int_<boost::multiprecision::number_kind_integer> > {
  typedef T NT;
  typedef CGAL::Tag_true Is_modularizable;
  typedef Residue Residue_type;

  struct Modular_image{
    Residue_type operator()(const NT& a){
      NT tmp(CGAL::mod(a,NT(Residue::get_current_prime())));
      return CGAL::Residue(tmp.template convert_to<int>());
    }
  };
  struct Modular_image_representative{
    NT operator()(const Residue_type& x){
      return NT(x.get_value());
    }
  };
};

// Split_double

template <class NT, class = boost::mpl::int_<boost::multiprecision::number_category<NT>::value> >
struct SD_boost_mp {
  void operator()(double d, NT &num, NT &den) const
  {
    num = d;
    den = 1;
  }
};

template <class NT>
struct SD_boost_mp <NT, boost::mpl::int_<boost::multiprecision::number_kind_integer> >
{
  void operator()(double d, NT &num, NT &den) const
  {
    std::pair<double, double> p = split_numerator_denominator(d);
    num = NT(p.first);
    den = NT(p.second);
  }
};


// Fraction_traits

template <class T, class = boost::mpl::int_<boost::multiprecision::number_category<T>::value> >
struct FT_boost_mp {
  typedef T Type;
  typedef Tag_false Is_fraction;
  typedef Null_tag Numerator_type;
  typedef Null_tag Denominator_type;
  typedef Null_functor Common_factor;
  typedef Null_functor Decompose;
  typedef Null_functor Compose;
};

template <class NT>
struct FT_boost_mp <NT, boost::mpl::int_<boost::multiprecision::number_kind_rational> > {
    typedef NT Type;

    typedef ::CGAL::Tag_true Is_fraction;
    typedef typename boost::multiprecision::component_type<NT>::type Numerator_type;
    typedef Numerator_type Denominator_type;

    typedef typename Algebraic_structure_traits< Numerator_type >::Gcd Common_factor;

    class Decompose {
    public:
        typedef Type first_argument_type;
        typedef Numerator_type& second_argument_type;
        typedef Denominator_type& third_argument_type;
        void operator () (
                const Type& rat,
                Numerator_type& num,
                Denominator_type& den) {
            num = numerator(rat);
            den = denominator(rat);
        }
    };

    class Compose {
    public:
        typedef Numerator_type first_argument_type;
        typedef Denominator_type second_argument_type;
        typedef Type result_type;
        Type operator ()(
                const Numerator_type& num ,
                const Denominator_type& den ) {
            return Type(num, den);
        }
    };
};

template <class Backend, boost::multiprecision::expression_template_option Eto>
struct Algebraic_structure_traits<boost::multiprecision::number<Backend, Eto> >
: AST_boost_mp <boost::multiprecision::number<Backend, Eto> > {};
template <class T1,class T2,class T3,class T4,class T5>
struct Algebraic_structure_traits<boost::multiprecision::detail::expression<T1,T2,T3,T4,T5> >
: Algebraic_structure_traits<typename boost::multiprecision::detail::expression<T1,T2,T3,T4,T5>::result_type > {};

template <class Backend, boost::multiprecision::expression_template_option Eto>
struct Real_embeddable_traits<boost::multiprecision::number<Backend, Eto> >
: RET_boost_mp <boost::multiprecision::number<Backend, Eto> > {};
template <class T1,class T2,class T3,class T4,class T5>
struct Real_embeddable_traits<boost::multiprecision::detail::expression<T1,T2,T3,T4,T5> >
: Real_embeddable_traits<typename boost::multiprecision::detail::expression<T1,T2,T3,T4,T5>::result_type > {};

template <class Backend, boost::multiprecision::expression_template_option Eto>
struct Modular_traits<boost::multiprecision::number<Backend, Eto> >
: MT_boost_mp <boost::multiprecision::number<Backend, Eto> > {};
template <class T1,class T2,class T3,class T4,class T5>
struct Modular_traits<boost::multiprecision::detail::expression<T1,T2,T3,T4,T5> >
: Modular_traits<typename boost::multiprecision::detail::expression<T1,T2,T3,T4,T5>::result_type > {};

template <class Backend, boost::multiprecision::expression_template_option Eto>
struct Fraction_traits<boost::multiprecision::number<Backend, Eto> >
: FT_boost_mp <boost::multiprecision::number<Backend, Eto> > {};
template <class T1,class T2,class T3,class T4,class T5>
struct Fraction_traits<boost::multiprecision::detail::expression<T1,T2,T3,T4,T5> >
: Fraction_traits<typename boost::multiprecision::detail::expression<T1,T2,T3,T4,T5>::result_type > {};

template <class Backend, boost::multiprecision::expression_template_option Eto>
struct Split_double<boost::multiprecision::number<Backend, Eto> >
: SD_boost_mp <boost::multiprecision::number<Backend, Eto> > {};
template <class T1,class T2,class T3,class T4,class T5>
struct Split_double<boost::multiprecision::detail::expression<T1,T2,T3,T4,T5> >
: Split_double<typename boost::multiprecision::detail::expression<T1,T2,T3,T4,T5>::result_type > {};


// Coercions

namespace internal { namespace boost_mp { BOOST_MPL_HAS_XXX_TRAIT_DEF(type); } }

template <class B1, boost::multiprecision::expression_template_option E1, class B2, boost::multiprecision::expression_template_option E2>
struct Coercion_traits<boost::multiprecision::number<B1, E1>, boost::multiprecision::number<B2, E2> >
{
  typedef boost::common_type<boost::multiprecision::number<B1, E1>, boost::multiprecision::number<B2, E2> > CT;
  typedef Boolean_tag<internal::boost_mp::has_type<CT>::value> Are_implicit_interoperable;
  // FIXME: the implicit/explicit answers shouldn't be the same...
  typedef Are_implicit_interoperable Are_explicit_interoperable;
  // FIXME: won't compile when they are not interoperable.
  typedef typename CT::type Type;
  struct Cast{
    typedef Type result_type;
    template <class U>
      Type operator()(const U& x) const {
	return Type(x);
      }
  };
};
// Avoid ambiguity with the specialization for <A,A> ...
template <class B1, boost::multiprecision::expression_template_option E1>
struct Coercion_traits<boost::multiprecision::number<B1, E1>, boost::multiprecision::number<B1, E1> >
{
  typedef boost::multiprecision::number<B1, E1> Type;
  typedef Tag_true Are_implicit_interoperable;
  typedef Tag_true Are_explicit_interoperable;
  struct Cast{
    typedef Type result_type;
    template <class U>
      Type operator()(const U& x) const {
	return Type(x);
      }
  };
};

template <class T1, class T2, class T3, class T4, class T5, class U1, class U2, class U3, class U4, class U5>
struct Coercion_traits <
boost::multiprecision::detail::expression<T1,T2,T3,T4,T5>,
boost::multiprecision::detail::expression<U1,U2,U3,U4,U5> >
: Coercion_traits <
typename boost::multiprecision::detail::expression<T1,T2,T3,T4,T5>::result_type,
typename boost::multiprecision::detail::expression<U1,U2,U3,U4,U5>::result_type>
{ };
// Avoid ambiguity with the specialization for <A,A> ...
template <class T1, class T2, class T3, class T4, class T5>
struct Coercion_traits <
boost::multiprecision::detail::expression<T1,T2,T3,T4,T5>,
boost::multiprecision::detail::expression<T1,T2,T3,T4,T5> >
: Coercion_traits <
typename boost::multiprecision::detail::expression<T1,T2,T3,T4,T5>::result_type,
typename boost::multiprecision::detail::expression<T1,T2,T3,T4,T5>::result_type>
{ };

template <class B, boost::multiprecision::expression_template_option E, class T1, class T2, class T3, class T4, class T5>
struct Coercion_traits<boost::multiprecision::number<B, E>, boost::multiprecision::detail::expression<T1,T2,T3,T4,T5> >
: Coercion_traits <
boost::multiprecision::number<B, E>,
typename boost::multiprecision::detail::expression<T1,T2,T3,T4,T5>::result_type>
{ };

template <class B, boost::multiprecision::expression_template_option E, class T1, class T2, class T3, class T4, class T5>
struct Coercion_traits<boost::multiprecision::detail::expression<T1,T2,T3,T4,T5>, boost::multiprecision::number<B, E> >
: Coercion_traits <
typename boost::multiprecision::detail::expression<T1,T2,T3,T4,T5>::result_type,
boost::multiprecision::number<B, E> >
{ };

// TODO: coercion with expressions, fix existing coercions
// (double -> rational is implicit only for 1.56+, see ticket #10082)
// The real solution would be to avoid specializing Coercion_traits for all pairs of number types and let it auto-detect what works, so only broken types need an explicit specialization.

// Ignore types smaller than long
#define CGAL_COERCE_INT(int) \
template <class B1, boost::multiprecision::expression_template_option E1> \
struct Coercion_traits<boost::multiprecision::number<B1, E1>, int> { \
  typedef boost::multiprecision::number<B1, E1> Type; \
  typedef Tag_true Are_implicit_interoperable; \
  typedef Tag_true Are_explicit_interoperable; \
  struct Cast{ \
    typedef Type result_type; \
    template <class U> Type operator()(const U& x) const { return Type(x); } \
  }; \
}; \
template <class B1, boost::multiprecision::expression_template_option E1> \
struct Coercion_traits<int, boost::multiprecision::number<B1, E1> > \
: Coercion_traits<boost::multiprecision::number<B1, E1>, int> {}

CGAL_COERCE_INT(short);
CGAL_COERCE_INT(int);
CGAL_COERCE_INT(long);
#undef CGAL_COERCE_INT

// Ignore bounded-precision rationals
#define CGAL_COERCE_FLOAT(float) \
template <class B1, boost::multiprecision::expression_template_option E1> \
struct Coercion_traits<boost::multiprecision::number<B1, E1>, float> { \
  typedef boost::multiprecision::number<B1, E1> Type; \
  typedef Boolean_tag<boost::multiprecision::number_category<Type>::value != boost::multiprecision::number_kind_integer> Are_implicit_interoperable; \
  typedef Are_implicit_interoperable Are_explicit_interoperable; \
  struct Cast{ \
    typedef Type result_type; \
    template <class U> Type operator()(const U& x) const { return Type(x); } \
  }; \
}; \
template <class B1, boost::multiprecision::expression_template_option E1> \
struct Coercion_traits<float, boost::multiprecision::number<B1, E1> > \
: Coercion_traits<boost::multiprecision::number<B1, E1>, float> {}

CGAL_COERCE_FLOAT(float);
CGAL_COERCE_FLOAT(double);
#undef CGAL_COERCE_FLOAT


} //namespace CGAL

#include <CGAL/BOOST_MP_arithmetic_kernel.h>

#endif // BOOST_VERSION
#endif

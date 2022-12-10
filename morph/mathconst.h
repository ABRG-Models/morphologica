/*!
 * Mathematical constants in the morph namespace.
 */
#pragma once

namespace morph {

    //! Square root of 3
    const float  SQRT_OF_3_F = 1.732050807568877293527446341505872366942805253810381f;
    const double SQRT_OF_3_D = 1.732050807568877293527446341505872366942805253810381;

    //! Root 3 over 2
    const float  SQRT_OF_3_OVER_2_F = 0.86602540378443864676372317075293618347140262690519f;
    const double SQRT_OF_3_OVER_2_D = 0.86602540378443864676372317075293618347140262690519;

    //! PI
    const float  PI_F = 3.141592653589793238462643383279502884197169399375106f;
    const double PI_D = 3.141592653589793238462643383279502884197169399375106;

    //! PI/2
    const float  PI_OVER_2_F = 1.570796326794896619231321691639751442098584699687553f;
    const double PI_OVER_2_D = 1.570796326794896619231321691639751442098584699687553;

    //! 3PI/2
    const float  PI_x3_OVER_2_F = 4.712388980384689857693965074919254326295754099062659f;
    const double PI_x3_OVER_2_D = 4.712388980384689857693965074919254326295754099062659;

    //! 2*PI
    const float  TWO_PI_F = 6.283185307179586476925286766559005768394338798750212f;
    const double TWO_PI_D = 6.283185307179586476925286766559005768394338798750212;

    //! Templated mathematical constants.
    //! Usage example: morph::mathconst<float>::pi_over_2
    //! T defaults to double, so morph::mathconst::pi gives pi in double precision.
    template <typename T=double>
    struct mathconst
    {
        static constexpr T sqrt_of_2 = T{1.414213562373095048801688724209698078569671875376948};
        static constexpr T one_over_root_2 = T{1}/sqrt_of_2;

        static constexpr T sqrt_of_3 = T{1.732050807568877293527446341505872366942805253810381};
        static constexpr T one_over_root_3 = T{1}/sqrt_of_3;
        static constexpr T two_over_root_3 = T{2}/sqrt_of_3;
        static constexpr T one_over_2_root_3 = T{1}/(T{2}*sqrt_of_3);
        static constexpr T root_3_over_2 = sqrt_of_3/T{2};

        static constexpr T pi = T{3.141592653589793238462643383279502884197169399375106};

        static constexpr T pi_over_2 = pi/T{2};
        static constexpr T three_pi_over_2 = pi*T{3}/T{2};

        static constexpr T pi_over_3 = pi/T{3};
        static constexpr T two_pi_over_3 = pi*T{2}/T{3};
        static constexpr T four_pi_over_3 = pi*T{4}/T{3};
        static constexpr T five_pi_over_3 = pi*T{5}/T{3};

        static constexpr T pi_over_4 = pi/T{4};
        static constexpr T pi_over_5 = pi/T{5};
        static constexpr T pi_over_6 = pi/T{6};
        static constexpr T pi_over_7 = pi/T{7};
        static constexpr T pi_over_8 = pi/T{8};
        static constexpr T pi_over_9 = pi/T{9};

        static constexpr T two_pi   = pi*T{2};
        static constexpr T three_pi = pi*T{3};
        static constexpr T four_pi  = pi*T{4};
        static constexpr T five_pi  = pi*T{5};
        static constexpr T six_pi   = pi*T{6};
        static constexpr T seven_pi = pi*T{7};
        static constexpr T eight_pi = pi*T{8};
        static constexpr T nine_pi  = pi*T{9};

        // multiplier to convert degrees to radians
        static constexpr T deg2rad  = two_pi/T{360};
        static constexpr T rad2deg  = T{360}/two_pi;
    };
}

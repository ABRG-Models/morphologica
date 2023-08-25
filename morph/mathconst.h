/*!
 * Mathematical constants in the morph namespace.
 */
#pragma once

namespace morph {

    //! Templated mathematical constants.
    //! Usage example: morph::mathconst<float>::pi_over_2
    //! T defaults to double, so morph::mathconst::pi gives pi in double precision.
    template <typename T=double>
    struct mathconst
    {
        static constexpr T root_2 = T{1.414213562373095048801688724209698078569671875376948};
        static constexpr T sqrt_of_2 = root_2;
        static constexpr T one_over_root_2 = T{1}/root_2;

        static constexpr T root_3 = T{1.732050807568877293527446341505872366942805253810381};
        static constexpr T sqrt_of_3 = root_3;
        static constexpr T one_over_root_3 = T{1}/root_3;
        static constexpr T two_over_root_3 = T{2}/root_3;
        static constexpr T one_over_2_root_3 = T{1}/(T{2}*root_3);
        static constexpr T root_3_over_2 = root_3/T{2};

        static constexpr T root_4 = T{2};
        static constexpr T root_5 = T{2.236067977499789696409173668731276235440618359612};
        static constexpr T root_6 = T{2.449489742783178098197284074705891391965947480657};
        static constexpr T root_7 = T{2.645751311064590590501615753639260425710259183082};
        static constexpr T root_8 = T{2.828427124746190097603377448419396157139343750754};
        static constexpr T root_9 = T{3};
        static constexpr T root_10 = T{3.162277660168379331998893544432718533719555139325};

        static constexpr T pi = T{3.141592653589793238462643383279502884197169399375106};

        static constexpr T pi_over_2 = pi/T{2};
        static constexpr T three_pi_over_2 = pi*T{3}/T{2};

        static constexpr T pi_over_3 = pi/T{3};
        static constexpr T two_pi_over_3 = pi*T{2}/T{3};
        static constexpr T four_pi_over_3 = pi*T{4}/T{3};
        static constexpr T five_pi_over_3 = pi*T{5}/T{3};

        static constexpr T pi_over_4 = pi/T{4};
        static constexpr T three_pi_over_4 = pi*T{3}/T{4};
        static constexpr T five_pi_over_4 = pi*T{5}/T{4};
        static constexpr T seven_pi_over_4 = pi*T{7}/T{4};

        static constexpr T pi_over_5 = pi/T{5};
        static constexpr T pi_over_6 = pi/T{6};
        static constexpr T pi_over_7 = pi/T{7};
        static constexpr T pi_over_8 = pi/T{8};
        static constexpr T pi_over_9 = pi/T{9};
        static constexpr T pi_over_10 = pi/T{10};
        static constexpr T pi_over_16 = pi/T{16};
        static constexpr T pi_over_18 = pi/T{18};
        static constexpr T pi_over_32 = pi/T{32};

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

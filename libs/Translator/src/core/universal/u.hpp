#pragma once

namespace ts::U {
    template <class T>
    struct Var {
        enum class linkage {
            INTERNAL,
            EXTERNAL
        };

        linkage linkageType;
        T value;
    };

    struct Operator {
        constexpr Operator() noexcept = default;

        virtual ~Operator() = default;

        virtual void operator()() noexcept = 0;
    };

    template <class T>
    struct Sum final : public Operator {
        Sum() = delete;
        explicit Sum(const Var<T>& var1, const Var<T>& var2) noexcept 
            : var1{ var1 },
              var2{ var2 }
        {}

        ~Sum() override = default;

        void operator()() noexcept override {
            result = var1 + var2;
        }

        Var<T> var1;
        Var<T> var2;
        Var<T> result;
    };

    template <class T>
    struct Mult final : public Operator {
        Mult() = delete;
        explicit Mult(const Var<T>& var1, const Var<T>& var2) noexcept 
            : var1{ var1 },
              var2{ var2 }
        {}

        ~Mult() override = default;

        void operator()() noexcept override {
            result = var1 * var2;
        }

        Var<T> var1;
        Var<T> var2;
        Var<T> result;
    };

    template <class T>
    struct Assign final : public Operator {
        Assign() = delete;
        explicit Assign(const Var<T>& src, const Var<T>& dst) noexcept 
            : src{ src },
              dst{ dst }
        {}

        ~Assign() override = default;

        void operator()() noexcept override {
            src = dst;
        }

        Var<T> src;
        Var<T> dst;
    };
}
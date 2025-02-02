#ifndef INC_WRAPPER_FLOAT_HPP_
#define INC_WRAPPER_FLOAT_HPP_

#include "WrapperBase.hpp"

class WrapperFloat
    : public WrapperBase
{
public:
    WrapperFloat(float& num)
        : value(num)
    {
        this->type = WrapperTypeT::WRAPPER_TYPE_FLOAT;
    }

    WrapperFloat& operator = (float num) {
        this->value = num;
        return (*this);
    }

protected:
    float get() const override {
        return this->value;
    }

    void set(float num) override {
        this->value = num;
    }

    const void* getPointer() const override {
        return nullptr;
    }

    void setPointer(const void* cptr) override {
        (void)cptr;
    }

private:
    float& value;
};

#endif /* INC_WRAPPER_FLOAT_HPP_ */

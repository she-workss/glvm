#pragma once

namespace glvm::Extra {
enum ETypes {
    eNONE,
    eUNSIGNED_INT,
    eINT,
    eDOUBLE,
    eFLOAT,
    eBOOL,
    eSHORT,
};

template<class T>
struct STypes {
    static const ETypes eType = eNONE;
};

template<>
struct STypes<unsigned int> {
    static const ETypes eType = eUNSIGNED_INT;
};

template<>
struct STypes<int> {
    static const ETypes eType = eINT;
};

template<>
struct STypes<double> {
    static const ETypes eType = eDOUBLE;
};

template<>
struct STypes<float> {
    static const ETypes eType = eFLOAT;
};

template<>
struct STypes<bool> {
    static const ETypes eType = eBOOL;
};

template<>
struct STypes<short> {
    static const ETypes eType = eSHORT;
};

template<typename T>
ETypes TypeEvaluator(T _Value) {
    switch (STypes<T>::eType) {
        case eUNSIGNED_INT:
            return eUNSIGNED_INT;
        case eINT:
            return eINT;
        case eDOUBLE:
            return eDOUBLE;
        case eFLOAT:
            return eFLOAT;
        case eBOOL:
            return eBOOL;
        case eSHORT:
            return eSHORT;
        default:
            return eNONE;
    }
}

} // namespace glvm::Extra

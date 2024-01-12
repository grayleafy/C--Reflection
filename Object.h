#ifndef Object_H
#define Object_H

#include <span>
#include <vector>
#include <iostream>
#include <functional>
#include <unordered_map>
#include <cinttypes>

#define REGISTER_CLASS(className)     \
class className;                \
Register register_##className = Register<className>();   

class Object;
class Field;
class Type;
template<typename T> Type& typeof_object();

template<typename T>
class Register{
public:
    Register();
};

class Field{
public:
    Type *type;           //类型
    std::size_t offset;  //数据偏移量
};

class Method{
public:
    void *func;

    Method(){
        func = nullptr;
    }

    Method(const Method &other){
        func = other.func;
    }

    template<typename R, typename C, typename... Args>
    Method(R (C::*classFun)(Args...));

    template<typename R, typename... Args>
    R invoke(Object *instance, Args&&... args){
        return (*static_cast<std::function<R(Object*, Args&&...)>*>(func))(instance, std::forward<Args>(args)...);
        //return func(instance, std::forward<Args>(args)...);
    }
};

class Type{
public:
    std::string name;                                    //类型名称
    std::function<Object*(void)> construct;              //构造函数
    std::function<void(void*&)> destroyData;             //析构函数
    std::function<void(void*&, void*&)> copyData;        //拷贝
    std::function<void(void*&, void*&)> moveData;        //移动
private:
    std::unordered_map<std::string, Field> fields;       //成员属性
    std::unordered_map<std::string, Method> methods;     //成员函数

    static std::unordered_map<std::string, Type*> types;
public:
    Type(){
        name = "nullType";
    }

    //根据名称查找类型
    static Type& getTypeByName(std::string typeName){
        if (types.find(typeName) == types.end()){
            throw std::runtime_error("can not find type " + typeName);
        }
        return *types[typeName];
    }

    //注册类型
    template<typename T>
    static void registerType(){
        Type *type = &typeof_object<T>();
        types[type->name] = type;
    }

    //创建实例
    Object* createInstance(){
        return construct();
    }

    //获取对象的属性
    Object& getField(Object *obj, const std::string &fieldName);
    Method& getMethod(const std::string &methodName){
        if (methods.find(methodName) == methods.end()){
            throw std::runtime_error("type " + name + " do not exsits method " + methodName);
        }
        return methods[methodName];
    }

    //插入属性
    void insertField(const std::string &fieldName, Type *type, std::size_t offset){
        fields[fieldName] = Field{type, offset};
    }

    //插入方法
    template<typename R, typename C, typename... Args>
    void insertMethod(const std::string &methodName, R (C::*classFun)(Args...)){
        methods[methodName] = Method(classFun);
    }

//禁用
private:
    Type(const Type &type) = delete;
    Type& operator == (const Type &type) = delete;
};

std::unordered_map<std::string, Type*> Type::types;

template<typename T>
Type& typeof_object(){
    static Type type;
    static bool isRegister = false;
    if (isRegister) return type;
    isRegister = true;
    type.name = typeid(T).name();
    type.construct = []()->Object*{
        T *t = new T();
        Object *obj = new Object(std::move(*t));
        return obj;
    };
    type.destroyData = [](void*& selfData){
        delete static_cast<T*>(selfData);
        //std::cout << type.name << ":析构" << std::endl;
    };
    type.copyData = [](void*& selfData, void*& otherData){
        selfData = new T(*static_cast<T*>(otherData));
        //std::cout << type.name << ":拷贝" << std::endl;
    };
    type.moveData = [](void*& selfData, void*& otherData){
        selfData = new T(std::move(*static_cast<T*>(otherData)));
        //std::cout << type.name << ":移动" << std::endl;
    };
    return type;
}

template<class T>
Register<T>::Register(){
    Type::registerType<T>();
}

class Object{
public:
    void *data;
    Type *type;

    template<typename T> friend Type& typeof_object();
    friend Type;

public:
    Object(Type *type, void *data): type(type), data(data){

    }

    //默认类型的拷贝和移动构造
    template<typename T>
    Object(T&& value){
        type = &typeof_object<std::decay_t<T>>();
        data = new std::decay_t<T>(std::forward<T>(value));
    }

    //拷贝构造
    Object(Object &obj){
        type = obj.type;
        type->copyData(data, const_cast<Object&>(obj).data);
    }

    //拷贝构造
    Object(const Object &obj){
        type = obj.type;
        type->copyData(data, const_cast<Object&>(obj).data);
    }

    //移动构造
    Object(Object &&obj){
        type = obj.type;
        type->moveData(data, obj.data);
    }

    //析构
    virtual ~Object(){
        type->destroyData(data);
    }

    //拆箱
    template<typename T>
    T& cast(){
        if (type != &typeof_object<T>()){
            throw std::runtime_error{"type mismatch"};
        }
        T* p = static_cast<T*>(data);
        return *p;
    }    

    //获取类型
    Type& getType(){
        return *type;
    }
};



//获取对象的属性
Object& Type::getField(Object *obj, const std::string &fieldName){
    if (fields.find(fieldName) == fields.end()){
        throw std::runtime_error("class " + name + " do not have field " + fieldName);
    }
    auto field = fields[fieldName];
    void* data = obj->data + field.offset;
    Object *res = new Object(field.type, data);
    return *res;
}

template<typename R, typename C, typename... Args>
Method::Method(R (C::*classFun)(Args...)){
    auto p = [classFun](Object *obj, Args... args)->R{
        return (static_cast<C*>(obj->data)->*classFun)(std::forward<Args>(args)...);
    };
    //std::function<R(Object *obj, Args... args)> f = p;
    func = new std::function<R(Object *obj, Args... args)>(p);
}

#endif
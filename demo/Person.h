#include "Object.h"
#include <string>


REGISTER_CLASS(Person)
class Person{
public:
    int age;
    std::string name;

    void addAge(int x){
        age += x;
    }
};

template<>
Type& typeof_object<Person>(){
    static Type type;
    static bool isRegister = false;
    if (isRegister) return type;
    isRegister = true;
    type.name = "Person";
    type.construct = []()->Object*{
        Person *t = new Person();
        Object *obj = new Object(std::move(*t));
        return obj;
    };
    type.destroyData = [](void*& selfData){
        delete static_cast<Person*>(selfData);
        //std::cout << type.name << ":析构" << std::endl;
    };
    type.copyData = [](void*& selfData, void*& otherData){
        selfData = new Person(*static_cast<Person*>(otherData));
        //std::cout << type.name << ":拷贝" << std::endl;
    };
    type.moveData = [](void*& selfData, void*& otherData){
        selfData = new Person(std::move(*static_cast<Person*>(otherData)));
        //std::cout << type.name << ":移动" << std::endl;
    };

    type.insertField("age", &typeof_object<int>(), offsetof(Person, age));
    type.insertField("name", &typeof_object<std::string>(), offsetof(Person, name));
    type.insertMethod("addAge", Person::addAge);
    return type;
}


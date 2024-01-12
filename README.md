# CPP-Reflection
c++反射的简易实现。根据类名获取类型、创建实例，根据名称获取类中属性和函数并调用
# 使用示例
使用宏定义注册要反射的类
~~~
REGISTER_CLASS(Person)
class Person{
public:
    int age;
    std::string name;

    void addAge(int x){
        age += x;
    }
};
~~~
元数据需要硬编码，否则使用模板定义的默认元数据。可以使用代码生成优化
~~~
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
~~~

之后可以直接在main函数中使用
~~~
int main(){
    Type &personType = Type::getTypeByName("Person");
    Object &p = *(personType.createInstance());

    cout << "age:" << p.getType().getField(&p, "age").cast<int>() << endl;
    cout << "name:" << p.getType().getField(&p, "name").cast<string>() << endl;

    Type &type = p.getType();
    Method &method = type.getMethod("addAge");
    method.invoke<void, int>(&p, 10);


    Person p2 = p.cast<Person>();
    cout << "age:" << p2.age << endl;
    cout << "name:" << p2.name << endl;

    
    {
        Object a = 100;
        Object b = 'b';

        Object c = std::move(b);

        
        cout << "a: type = " << a.getType().name << ", val = " << a.cast<int>() << endl;
        cout << "c: type = " << c.getType().name << ", val = " << c.cast<char>() << endl;

    }
}
~~~

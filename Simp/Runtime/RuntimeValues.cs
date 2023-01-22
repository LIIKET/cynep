using Newtonsoft.Json.Linq;
using Simp.Runtime;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Simp
{
    public enum ValueType
    {
        Null,
        Number,
        Boolean,

        Type,
        Object,
        Property,

        NativeFunction
    }
    public class RuntimeValue
    {
        public ValueType Type;
    }

    public class NullValue : RuntimeValue
    {
        public string? Value;

        public NullValue()
        {
            Type = ValueType.Null;
            Value = null;
        }
    }

    public class BooleanValue : RuntimeValue
    {
        public bool Value;

        public BooleanValue(bool value)
        {
            Type = ValueType.Boolean;
            Value = value;
        }
    }

    public class NumberValue: RuntimeValue
    {
        public long Value;

        public NumberValue(long value)
        {
            Type = ValueType.Number;
            Value = value;
        }
    }

    public class PropertyValue : RuntimeValue
    {
        public RuntimeValue Value;

        public PropertyValue(RuntimeValue value)
        {
            Type = ValueType.Property;
            Value = value;
        }
    }

    // Represents a type declaration
    public class TypeValue : RuntimeValue 
    {
        public string Name;
        public List<PropertyDeclaration> Properties;


        public TypeValue(string name, List<PropertyDeclaration> properties)
        {
            Type = ValueType.Type;
            Name= name;
            Properties = properties;

        }
    }

    public class ObjectValue : RuntimeValue
    {
        public string TypeName;
        public Dictionary<string, RuntimeValue> Properties;
        public Scope Scope;

        public ObjectValue(string typeName, Scope scope)
        {
            Type = ValueType.Object;
            TypeName = typeName;
            Properties = new Dictionary<string, RuntimeValue>();
            Scope = scope;
        }
    }
}

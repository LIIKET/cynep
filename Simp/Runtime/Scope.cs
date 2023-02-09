using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Linq;

namespace Simp.Runtime
{
    public class Scope
    {
        [JsonIgnore]
        public Scope? Parent;
        public Dictionary<string, RuntimeValue> Variables;

        public Scope(Scope? parent)
        {
            Variables = new Dictionary<string, RuntimeValue>();
            Parent = parent;

            // Setup global scope
            if (parent == null)
            {
                DeclareVariable("true", new BooleanValue(true));
                DeclareVariable("false", new BooleanValue(false));
                DeclareVariable("null", new NullValue());
            }
        }

        public RuntimeValue DeclareVariable(string name, RuntimeValue value)
        {
            if (Variables.ContainsKey(name))
            {
                throw new Exception($"Cannot declare variable {name}. It already exists in scope.");
            }
            else
            {
                Variables.Add(name, value);
            }

            return value;
        }

        public RuntimeValue AssignVariable(string name, RuntimeValue value)
        {
            var scope = ResolveScope(name);

            scope.Variables[name] = value;
            return value;
        }



        public RuntimeValue GetVariable(string name)
        {
            var scope = ResolveScope(name);
            return scope.Variables[name];
        }

        public Scope ResolveScope(string variableName)
        {
            if (Variables.ContainsKey(variableName))
            {
                return this;
            }
            else if (Parent != null)
            {
                return Parent.ResolveScope(variableName);
            }
            else
            {
                throw new Exception($"Cannot resolve variable {variableName}. It doesn't exist in scope.");
            }
        }
    }
}

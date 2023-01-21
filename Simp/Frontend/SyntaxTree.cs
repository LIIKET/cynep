using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;

namespace Simp
{
    public enum NodeType
    {
        // Statements
        Program,
        VariableDeclaration,
        TypeDeclaration,
        PropertyDeclaration,

        // Expression
        AssignmentExpression,
        BinaryExpression,
        ComparisonExpression,
        MemberExpression,
        CallExpression,

        // Literals
        NumericLiteral,
        NullLiteral,

        Identifier,
    }
    public class Statement
    {
        [JsonProperty(Order = -2)]
        public NodeType Type;
    }

    public class Program : Statement
    {
        public List<Statement> Body = new List<Statement>();

        public Program()
        {
            Type = NodeType.Program;
        }
    }

    public class VariableDeclaration : Statement
    {
        public string Name;
        public Expression? Value;

        public VariableDeclaration(string name, Expression? value)
        {
            Name = name;
            Value = value;
            Type = NodeType.VariableDeclaration;
        }
    }

    public class TypeDeclaration : Statement
    {
        public string Name;
        public List<PropertyDeclaration> Properties = new List<PropertyDeclaration>();

        public TypeDeclaration(string name)
        {
            Name = name;
            Type = NodeType.TypeDeclaration;
        }
    }

    public class PropertyDeclaration : Statement
    {
        public string Name;

        public PropertyDeclaration(string name, Expression? value)
        {
            Name = name;
            Type = NodeType.PropertyDeclaration;
        }
    }

    public class CallExpression : Expression
    {
        public Expression Callee;
        public List<Expression> Args;

        public CallExpression(Expression callee, List<Expression> args)
        {
            Callee = callee;
            Args = args;
            Type = NodeType.CallExpression;
        }
    }

    public class MemberExpression : Expression
    {
        public Expression Object;
        public Expression Property;

        public MemberExpression(Expression @object, Expression property)
        {
            Object = @object;
            Property = property;
            Type = NodeType.MemberExpression;
        }
    }

    public class AssignmentExpression : Expression
    {
        public Expression Assignee;
        public Expression Value;

        public AssignmentExpression(Expression assignee, Expression value)
        {
            Assignee = assignee;
            Value = value;
            Type = NodeType.AssignmentExpression;
        }
    }

    public class Expression : Statement
    {

    }

    public class BinaryExpression : Expression
    {
        public string Operator;
        public Expression Left;
        public Expression Right;

        public BinaryExpression(Expression left, string @operator, Expression right)
        {
            Left = left;
            Right = right;
            Operator = @operator;
            Type = NodeType.BinaryExpression;
        }
    }

    public class ComparisonExpression : Expression
    {
        public string Operator;
        public Expression Left;
        public Expression Right;

        public ComparisonExpression(Expression left, string @operator, Expression right)
        {
            Left = left;
            Right = right;
            Operator = @operator;
            Type = NodeType.ComparisonExpression;
        }
    }

    public class Identifier : Expression
    {
        public string Name;

        public Identifier(string name)
        {
            Name = name;
            Type = NodeType.Identifier;
        }
    }

    public class NumericLiteral : Expression
    {
        public long Value;

        public NumericLiteral(long value)
        {
            Value = value;
            Type = NodeType.NumericLiteral;
        }
    }

    public class NullLiteral : Expression
    {
        public string Value;

        public NullLiteral()
        {
            Value = "null";
            Type = NodeType.NullLiteral;
        }
    }
}

using System.Linq.Expressions;

namespace Simp.Runtime
{
    public class Interpreter
    {
        public RuntimeValue Evaluate(Statement statement, Scope scope)
        {
            // TODO: Traverse the whole tree for type declarations / global functions to put in global scope

            switch (statement.Type)
            {
                case NodeType.NumericLiteral:
                    {
                        return new NumberValue(((NumericLiteral)statement).Value);
                    }
                case NodeType.NullLiteral:
                    {
                        return new NullValue();
                    }
                case NodeType.Program:
                    {
                        return EvaluateProgram((Program)statement, scope);
                    }
                case NodeType.VariableDeclaration:
                    {
                        return EvaluateVariableDeclaration((VariableDeclaration)statement, scope);
                    }
                case NodeType.TypeDeclaration:
                    {
                        return EvaluateTypeDeclaration((TypeDeclaration)statement, scope);
                    }
                case NodeType.AssignmentExpression:
                    {
                        return EvaluateVariableAssignment((AssignmentExpression)statement, scope);
                    }
                case NodeType.Identifier:
                    {
                        return EvaluateIdentifier((Identifier)statement, scope);
                    }
                case NodeType.BinaryExpression:
                    {
                        return EvaluateBinaryExpression((BinaryExpression)statement, scope);
                    }
                case NodeType.ComparisonExpression:
                    {
                        return EvaluateLogicalExpression((ComparisonExpression)statement, scope);
                    }
                case NodeType.CallExpression:
                    {
                        return EvaluateCallExpression((CallExpression)statement, scope);
                    }
                case NodeType.MemberExpression:
                    {
                        return EvaluateMemberExpression((MemberExpression)statement, scope);
                    }
                default:
                    throw new Exception($"Cannot interpret syntax tree node of type {statement.Type}");
            }
        }

        public RuntimeValue ResolveForMemberExpression(MemberExpression statement, Scope scope, bool resolveContainer)
        {
            // TODO: Does it really have to be this complicated?

            var currentExpression = statement;
            var memberKeys = new List<string>();

            while (currentExpression.Type != NodeType.Identifier)
            {
                memberKeys.Add(currentExpression.Property.Name);

                if (currentExpression.Object.Type != NodeType.Identifier)
                {
                    currentExpression = currentExpression.Object as MemberExpression;
                }
                else
                {
                    memberKeys.Add((currentExpression.Object as Identifier).Name);
                    break;
                }
            }

            memberKeys.Reverse();

            // First item is always variable
            var prop = scope.GetVariable(memberKeys.First());

            memberKeys.RemoveAt(0);

            foreach (var memberKey in memberKeys)
            {
                if (resolveContainer)
                {
                    if ((prop as ObjectValue).Properties[memberKey].Type == ValueType.Object)
                    {
                        prop = (prop as ObjectValue).Properties[memberKey];
                    }
                }
                else
                {
                    prop = (prop as ObjectValue).Properties[memberKey];
                }
            }

            return prop;
        }

        public RuntimeValue EvaluateMemberExpression(MemberExpression statement, Scope scope)
        {
            var member = ResolveForMemberExpression(statement, scope, false);

            return member;
        }

        public RuntimeValue EvaluateCallExpression(CallExpression statement, Scope scope)
        {
            List<RuntimeValue> args = new List<RuntimeValue>();

            foreach (var arg in statement.Args)
            {
                args.Add(Evaluate(arg, scope));
            }

            // TODO: Fix! This is trash. Handle native functions properly

            var type = Evaluate(statement.Args[0], scope) as TypeValue;
            var val = new ObjectValue(type.Name, new Scope(scope));

            foreach (var prop in type.Properties)
            {
                val.Properties.Add(prop.Name, new NumberValue(12));

            }

            return val;
        }

        public RuntimeValue EvaluateTypeDeclaration(TypeDeclaration declaration, Scope scope)
        {
            var type = new TypeValue(declaration.Name, declaration.Properties);

            return scope.DeclareVariable(declaration.Name, type);
        }

        public RuntimeValue EvaluateVariableDeclaration(VariableDeclaration variableDeclaration, Scope scope)
        {
            var value = variableDeclaration.Value == null ? new NullValue() : Evaluate(variableDeclaration.Value, scope);
            return scope.DeclareVariable(variableDeclaration.Name, value);
        }

        public RuntimeValue EvaluateVariableAssignment(AssignmentExpression assignmentExpression, Scope scope)
        {
            var value = assignmentExpression.Value == null ? new NullValue() : Evaluate(assignmentExpression.Value, scope);

            // Handles member expressions as assignee. Example: {obj}.{member} = {erpression}
            if (assignmentExpression.Assignee.Type == NodeType.MemberExpression)
            {
                var memberParent = ResolveForMemberExpression(assignmentExpression.Assignee as MemberExpression, scope, true) as ObjectValue;
                var assigneeMember = (assignmentExpression.Assignee as MemberExpression).Property.Name;

                memberParent.Properties[assigneeMember] = value;

                return value;
            }

            // Handle standard variable assignment
            var name = (assignmentExpression.Assignee as Identifier)!.Name;
            
            return scope.AssignVariable(name, value);
        }

        public RuntimeValue EvaluateProgram(Program program, Scope scope)
        {
            RuntimeValue lastResult = new NullValue();

            foreach (var statement in program.Body)
            {
                lastResult = Evaluate(statement, scope);
            }

            return lastResult;
        }

        public RuntimeValue EvaluateIdentifier(Identifier identifier, Scope scope)
        {
            var value = scope.GetVariable(identifier.Name);

            return value;
        }

        public RuntimeValue EvaluateBinaryExpression(BinaryExpression binaryExpression, Scope scope)
        {
            var left = Evaluate(binaryExpression.Left, scope);
            var right = Evaluate(binaryExpression.Right, scope);

            if (left.Type == ValueType.Number && right.Type == ValueType.Number)
            {
                return EvaluateNumericBinaryExpression(left as NumberValue, binaryExpression.Operator, right as NumberValue, scope);
            }

            // One or both are null
            return new NullValue();
        }

        public RuntimeValue EvaluateNumericBinaryExpression(NumberValue left, string @operator, NumberValue right, Scope scope)
        {
            switch (@operator)
            {
                case "+":
                    {
                        return new NumberValue(left.Value + right.Value);
                    }
                case "-":
                    {
                        return new NumberValue(left.Value - right.Value);
                    }
                case "*":
                    {
                        return new NumberValue(left.Value * right.Value);
                    }
                case "/":
                    {
                        return new NumberValue(left.Value / right.Value);
                    }
                case "%":
                    {
                        return new NumberValue(left.Value % right.Value);
                    }
                default:
                    throw new Exception($"Invalid operator in interpreter: {@operator}");
            }
        }

        public RuntimeValue EvaluateLogicalExpression(ComparisonExpression binaryExpression, Scope scope)
        {
            var left = Evaluate(binaryExpression.Left, scope);
            var right = Evaluate(binaryExpression.Right, scope);

            if (left.Type == ValueType.Number && right.Type == ValueType.Number)
            {
                return EvaluateNumericLogicalExpression(left as NumberValue, binaryExpression.Operator, right as NumberValue, scope);
            }

            // One or both are null
            return new NullValue();
        }

        public RuntimeValue EvaluateNumericLogicalExpression(NumberValue left, string @operator, NumberValue right, Scope scope)
        {
            switch (@operator)
            {
                case "==":
                    {
                        return new BooleanValue(left.Value == right.Value);
                    }
                case "!=":
                    {
                        return new BooleanValue(left.Value != right.Value);
                    }
                case ">":
                    {
                        return new BooleanValue(left.Value > right.Value);
                    }
                case ">=":
                    {
                        return new BooleanValue(left.Value >= right.Value);
                    }
                case "<":
                    {
                        return new BooleanValue(left.Value < right.Value);
                    }
                case "<=":
                    {
                        return new BooleanValue(left.Value <= right.Value);
                    }
                default:
                    throw new Exception($"Invalid operator in interpreter: {@operator}");
            }
        }

    }


}

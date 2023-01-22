using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics.Metrics;
using System.Linq;
using System.Linq.Expressions;
using System.Text;
using System.Threading.Tasks;

namespace Simp
{
    public class Parser
    {
        private List<Token> tokens = new List<Token>();

        public Program BuildSyntaxTree(List<Token> tokens)
        {
            this.tokens = tokens;

            var program = new Program();

            while (!EndOfFile())
            {
                program.Body.Add(ParseStatement());
            }

            return program;
        }

        private Statement ParseStatement()
        {
            switch (Current().Type)
            {
                case TokenType.Let:
                    {
                        return ParseVariableDeclaration();
                    }
                case TokenType.Type:
                    {
                        return ParseTypeDeclaration();
                    }
                default:
                    {
                        return ParseExpression();
                    }
            }
            
        }

        // var {identifier} : {type} = {expression};
        // var {identifier};
        private VariableDeclaration ParseVariableDeclaration()
        {
            var letToken = Consume(); // Get rid of var token
            var identifier = Consume(TokenType.Identifier, "Let keyword should be followed by an identifier.");
            
            if(Current().Type == TokenType.Semicolon)
            {
                Consume(); // Get rid of semicolon
                return new VariableDeclaration(identifier.Value, null);
            }
            else
            {
                Consume(TokenType.Assignment, "Identifier in var declaration should be followed by an equals token.");
                var expression = ParseExpression();
                Consume(TokenType.Semicolon, "Variable declaration must end with semicolon.");
                return new VariableDeclaration(identifier.Value, expression);
            }
        }

        private TypeDeclaration ParseTypeDeclaration()
        {
            var typeToken = Consume(); // Get rid of type token
            var identifier = Consume(TokenType.Identifier, "Type keyword should be followed by an identifier.");
            Consume(TokenType.Assignment, "Error in type declaration");
            Consume(TokenType.OpenBrace, "Error in type declaration");

            var typeDeclaration = new TypeDeclaration(identifier.Value);

            while (!EndOfFile() && Current().Type != TokenType.CloseBrace)
            {
                var propertyIdentifier = Consume(TokenType.Identifier, "Error in type declaration");
                Consume(TokenType.Semicolon, "Error in type declaration");
                var property = new PropertyDeclaration(propertyIdentifier.Value, null);
                typeDeclaration.Properties.Add(property);
            }

            Consume(TokenType.CloseBrace, "Error in type declaration");

            return typeDeclaration;
        }

        private Expression ParseExpression()
        {
            return ParseAssignmentExpression();
        }


        // -- Orders of prescidence (highest last, just like execution order) --

        // AssignmentExpression             [X]
        // Object
        // LogicalExpression                
        // ComparisonExpression             [X]
        // AdditiveExpression               [X]
        // MiltiplicativeExpression         [X]
        // UnaryExpression
        // FunctionCall                     [/]
        // MemberExpression                 [X]
        // PrimaryExpression                [X]



        private Expression ParseAssignmentExpression()
        {
            var left = ParseComparisonExpression();

            if (Current().Type == TokenType.Assignment)
            {
                Consume();
                var value = ParseComparisonExpression();
                Consume(TokenType.Semicolon, "Variable assignment must end with semicolon.");
                return new AssignmentExpression(left, value);
            }

            return left;
        }

        private Expression ParseComparisonExpression()
        {
            var left = ParseAdditiveExpression();
            var test = Current().Value;
            while (Current().Value == "==" || Current().Value == "!="  || Current().Value == ">" || Current().Value == ">=" || Current().Value == "<" || Current().Value == "<=")
            {
                var _operator = Consume().Value;
                var right = ParseAdditiveExpression();
                left = new ComparisonExpression(left, _operator, right);

                // TODO: Should be possible to do type checking here
            }

            return left;
        }

        private Expression ParseAdditiveExpression()
        {
            var left = ParseMultiplicativeExpression();

            while (Current().Value == "+" || Current().Value == "-")
            {
                var _operator = Consume().Value;
                var right = ParseMultiplicativeExpression();
                left = new BinaryExpression(left, _operator, right);

                // TODO: Should be possible to do type checking here
            }

            return left;
        }

        private Expression ParseMultiplicativeExpression()
        {
            var left = ParseCallMemberExpression();

            while (Current().Value == "*" || Current().Value == "/" || Current().Value == "%")
            {
                var _operator = Consume().Value;
                var right = ParseCallMemberExpression();
                left = new BinaryExpression(left, _operator, right);
            }

            return left;
        }

        /*
         *
         *  THESE ARE GROUPED
         *
         * */
        private Expression ParseCallMemberExpression()
        {
            var member = ParseMemberExpression();

            if (Current().Type == TokenType.OpenParen)
            {
                return ParseCallExpression(member);
            }

            return member;
        }

        private CallExpression ParseCallExpression(Expression caller)
        {
            var callExpr = new CallExpression(caller, ParseArgs());

            if (Current().Type == TokenType.OpenParen)
            {
                callExpr = ParseCallExpression(callExpr);
            }

            return callExpr;
        }

        private List<Expression> ParseArgs()
        {
            Consume(TokenType.OpenParen);
            var args = new List<Expression>();  

            if(Current().Type == TokenType.CloseParen)
            {
                args = new List<Expression>();
            }
            else
            {
                args = ParseArgumentsList();
            }

            Consume(TokenType.CloseParen);

            return args;
        }

        private List<Expression> ParseArgumentsList()
        {
            var args = new List<Expression>();
            args.Add(ParseAssignmentExpression());

            while (Current().Type == TokenType.Comma)
            {
                Consume();
                args.Add(ParseAssignmentExpression());
            }

            return args;
        }

        private Expression ParseMemberExpression()
        {
            var obj = ParsePrimaryExpression();

            while (Current().Type == TokenType.Dot)
            {
                Consume(); // Eat dot
                var property = ParsePrimaryExpression();

                if (property.Type != NodeType.Identifier)
                {
                    throw new Exception("Dot operator requires identifier on right hand");
                }

                obj = new MemberExpression(obj, property as Identifier);
            }

            return obj;
        }

        /*
         *
         *  END THESE ARE GROUPED
         *
         * */

        private Expression ParsePrimaryExpression()
        {
            var token = Current();

            switch (token.Type) { 
                case TokenType.Identifier:
                    {
                        return new Identifier(Consume().Value);
                    }
                case TokenType.Number:
                    {
                        return new NumericLiteral(long.Parse(Consume().Value));
                    }
                case TokenType.Null:
                    {
                        Consume();
                        return new NullLiteral();
                    }
                case TokenType.OpenParen:
                    {
                        Consume(); // Throw away open paren
                        var expression = this.ParseExpression();
                        Consume(TokenType.CloseParen); // Throw away close paren

                        return expression;
                    }
                default:
                    {
                        throw new Exception($"Unexpected token in parser: \"{Current().Value}\"");
                    }
            }
        }

        private Token Current() { 
            return tokens.First(); 
        }

        private Token Consume(TokenType? expected = null, string? error = null)
        {
            var token = tokens.First();

            if(expected != null && token.Type != expected)
            {
                var additiveError = error == null ? "" : error;
                throw new Exception($"Parser expected \"{expected}\" but got \"{token.Type}\". {additiveError}");
            }

            tokens.RemoveAt(0);
            return token;
        }

        private bool EndOfFile()
        {
            return tokens.First().Type == TokenType.EndOfFile;
        }
    }
}

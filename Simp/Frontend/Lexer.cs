using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Reflection.Metadata;
using System.Text;
using System.Threading.Tasks;

namespace Simp
{
    public enum TokenType
    {
        // Literals
        Number,
        Identifier,

        // Keywords
        Let,
        Null,
        Type,

        // Logical operators
        ComparisonOperator, // == > < != etc

        // Grouping / Operators
        BinaryOperator, // TODO: Byt namn till ArithmeticOperator. + - / * etc 
        Assignment,
        Semicolon,
        OpenParen,
        CloseParen,
        OpenBrace,
        CloseBrace,
        Comma,
        Dot,
        EndOfFile
    }

    //{expr} == {expr}

    public class Token
    {
        public TokenType Type;
        public string Value;

        public Token(TokenType type, string value)
        {
            Value = value;
            Type = type;
        }
    }

    public class Lexer
    {
        private Dictionary<string, TokenType> keywords = new Dictionary<string, TokenType>
        {
            {"type", TokenType.Type},
            {"var", TokenType.Let},
            {"null", TokenType.Null},
        };

        private bool IsLetter(char src)
        {
            return char.IsLetter(src);
        }

        private bool IsDigit(char src)
        {
            return char.IsDigit(src);
        }

        private bool IsSpecial(char src)
        {
            return !char.IsLetterOrDigit(src) && !IsSkippable(src);
        }
        private bool IsSkippable(char src)
        {
            return src == ' ' || src == '\n' || src == '\t' || src == '\r';
        }

        public List<Token> Tokenize(string source)
        {
            var tokens = new List<Token>();
            var characters = new Stack<char>();

            // Build stack with reverse loop
            for (int i = source.Length - 1; i >= 0; i--)
            {
                characters.Push(source[i]);
            }

            // Build tokens until end of file
            while (characters.Count > 0)
            {
                switch (characters.Peek())
                {
                    case '+':
                    case '-':
                    case '*':
                    case '/':
                    case '%':
                        {
                            tokens.Add(new Token(TokenType.BinaryOperator, characters.Pop().ToString()));
                            break;
                        }
                    case '(':
                        {
                            tokens.Add(new Token(TokenType.OpenParen, characters.Pop().ToString()));
                            break;
                        }
                    case ')':
                        {
                            tokens.Add(new Token(TokenType.CloseParen, characters.Pop().ToString()));
                            break;
                        }
                    case '{':
                        {
                            tokens.Add(new Token(TokenType.OpenBrace, characters.Pop().ToString()));
                            break;
                        }
                    case '}':
                        {
                            tokens.Add(new Token(TokenType.CloseBrace, characters.Pop().ToString()));
                            break;
                        }
                    case ',':
                        {
                            tokens.Add(new Token(TokenType.Comma, characters.Pop().ToString()));
                            break;
                        }
                    case '.':
                        {
                            tokens.Add(new Token(TokenType.Dot, characters.Pop().ToString()));
                            break;
                        }
                    case '!':
                        {
                            characters.Pop();

                            if (characters.Count > 0 && characters.Peek() == '=')
                            {
                                characters.Pop();
                                tokens.Add(new Token(TokenType.ComparisonOperator, "!="));
                            }
                            else
                            {
                                // TODO: Some kind of negation operator
                            }

                            break;
                        }
                    case '>':
                        {
                            characters.Pop();

                            if (characters.Count > 0 && characters.Peek() == '=')
                            {
                                characters.Pop();
                                tokens.Add(new Token(TokenType.ComparisonOperator, ">="));

                            }
                            else
                            {
                                tokens.Add(new Token(TokenType.ComparisonOperator, ">"));
                            }

                            break;
                        }
                    case '<':
                        {
                            characters.Pop();

                            if (characters.Count > 0 && characters.Peek() == '=')
                            {
                                characters.Pop();
                                tokens.Add(new Token(TokenType.ComparisonOperator, "<="));

                            }
                            else
                            {
                                tokens.Add(new Token(TokenType.ComparisonOperator, "<"));
                            }

                            break;
                        }
                    case '=':
                        {
                            characters.Pop();
                            
                            if (characters.Count > 0 && characters.Peek() == '=')
                            {
                                characters.Pop();
                                tokens.Add(new Token(TokenType.ComparisonOperator, "=="));
                                
                            }
                            else
                            {
                                tokens.Add(new Token(TokenType.Assignment, "="));
                            }

                            break;
                        }
                    case ';':
                        {
                            tokens.Add(new Token(TokenType.Semicolon, characters.Pop().ToString()));
                            break;
                        }
                    default:
                        {
                            // Handle multicharacter tokens here

                            if (IsDigit(characters.Peek()))
                            {
                                // Create number token

                                var number = string.Empty;

                                while (characters.Count > 0 && IsDigit(characters.Peek()))
                                {
                                    number += characters.Pop();
                                }

                                tokens.Add(new Token(TokenType.Number, number));
                            }
                            else if (IsLetter(characters.Peek()))
                            {
                                var identifier = string.Empty;

                                while (characters.Count > 0 && IsLetter(characters.Peek()))
                                {
                                    identifier += characters.Pop();
                                }

                                // Check for reserved keywords
                                if(keywords.TryGetValue(identifier, out var type))
                                {
                                    tokens.Add(new Token(type, identifier));
                                }
                                else
                                {
                                    tokens.Add(new Token(TokenType.Identifier, identifier));
                                }
                            }
                            else if (IsSkippable(characters.Peek()))
                            {
                                characters.Pop();
                            }
                            else
                            {
                                throw new Exception($"Unrecognized character found in source: {characters.Peek()}");
                            }

                            break;
                        }
                }
            }

            tokens.Add(new Token(TokenType.EndOfFile, "EndOfFile"));

            return tokens;
        }
    }
}

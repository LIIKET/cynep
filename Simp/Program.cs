using Newtonsoft.Json;
using Newtonsoft.Json.Converters;
using Simp;
using Simp.Runtime;


var useRepl = false;

var lexer = new Lexer();
var parser = new Parser();
var interpreter = new Interpreter();
var scope = new Scope(null);

var repl = new Repl(lexer, parser, interpreter, scope);

if (useRepl)
{
    repl.Start();
}
else
{
    Console.WriteLine(Repl.greeting);

    var code = File.ReadAllText("Tests/program.cynep");
    var tokens = lexer.Tokenize(code);

    //Console.WriteLine(JsonConvert.SerializeObject(tokens, Formatting.Indented, new JsonConverter[] { new StringEnumConverter() }));

    var syntaxTree = parser.BuildSyntaxTree(tokens);

    Console.WriteLine(JsonConvert.SerializeObject(syntaxTree, Formatting.Indented, new JsonConverter[] { new StringEnumConverter() }));

    var result = interpreter.Evaluate(syntaxTree, scope);

    Console.WriteLine(JsonConvert.SerializeObject(result, Formatting.Indented, new JsonConverter[] { new StringEnumConverter() }));
}

public class Repl
{
    private Lexer lexer;
    private Parser parser;
    private Interpreter interpreter;
    private Scope scope;
    public static string greeting = @"
           _____   ___  _ ___ ___          ___  ___    ___ _   _  ___ _  _____ _  _  ___  __      _____ _  _ 
          / __\ \ / / \| | __| _ \  ___   / __|/ _ \  | __| | | |/ __| |/ /_ _| \| |/ __| \ \    / /_ _| \| |
         | (__ \ V /| .` | _||  _/ |___| | (_ | (_) | | _|| |_| | (__| ' < | || .` | (_ |  \ \/\/ / | || .` |
          \___| |_| |_|\_|___|_|          \___|\___/  |_|  \___/ \___|_|\_\___|_|\_|\___|   \_/\_/ |___|_|\_|                                                                                                
        ";

    public Repl(Lexer lexer, Parser parser, Interpreter interpreter, Scope scope)
    {
        this.lexer = lexer;
        this.parser = parser;
        this.interpreter = interpreter;
        this.scope = scope;
    }

    public void Start()
    {
        Console.WriteLine(greeting);

        var print_ast = false;
        var print_lexer = true;
        var print_eval = false;

        while (true)
        {
            Console.Write("> ");
            var code = Console.ReadLine();

            switch (code)
            {
                case "ast on":
                    print_ast = true; continue;
                case "ast off":
                    print_ast = false; continue;
                case "tokens on":
                    print_lexer = true; continue;
                case "tokens off":
                    print_lexer = false; continue;
                case "eval on":
                    print_eval = true; continue;
                case "eval off":
                    print_eval = false; continue;
                default:
                    break;
            }

            try
            {
                var tokens = lexer.Tokenize(code);

                if (print_lexer)
                {
                    Console.WriteLine("--- LEXER TOKEN OUTPUT ---");
                    Console.WriteLine(JsonConvert.SerializeObject(tokens, Formatting.Indented, new JsonConverter[] { new StringEnumConverter() }));
                }

                var syntaxTree = parser.BuildSyntaxTree(tokens);

                if (print_ast)
                {
                    Console.WriteLine("--- ABSTRACT SYNTAX TREE ---");
                    Console.WriteLine(JsonConvert.SerializeObject(syntaxTree, Formatting.Indented, new JsonConverter[] { new StringEnumConverter() }));
                }

                var result = interpreter.Evaluate(syntaxTree, scope);

                if (print_eval)
                {
                    Console.WriteLine("--- EXECUTION RESULT ---");
                    Console.WriteLine(JsonConvert.SerializeObject(result, Formatting.Indented, new JsonConverter[] { new StringEnumConverter() }));
                }
            }
            catch (Exception e)
            {
                Console.WriteLine(e.Message);
            }
        }
    }
}

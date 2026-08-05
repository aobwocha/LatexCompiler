package main

import (
	"fmt"
	"os"
	"strings"
)

func main() {
	if len(os.Args) < 2 {
		fmt.Println("Pass LaTEX file")
		return
	}

	filePath := os.Args[1]

	tokens, err := Lex(filePath)
	if err != nil {
		fmt.Printf("Lexing error: %v\n", err)
		return
	}

	parser := NewParser(tokens)
	astNodes := parser.parseNodeList()

	for i, node := range astNodes {
		fmt.Printf("[%d] ", i)
		printAST(node, 0)
	}
}

func printAST(node Node, indent int) {
	prefix := strings.Repeat("  ", indent)

	switch n := node.(type) {
	case *TextNode:
		cleanVal := strings.ReplaceAll(n.Value, "\n", "\\n")
		fmt.Printf("%sTextNode: %q\n", prefix, cleanVal)

	case *CommandNode:
		fmt.Printf("%sCommandNode: %s\n", prefix, n.Name)

		if len(n.OptArgs) > 0 {
			fmt.Printf("%s  OptArgs [...]:\n", prefix)
			for _, child := range n.OptArgs {
				printAST(child, indent+2)
			}
		}

		if len(n.ReqArgs) > 0 {
			fmt.Printf("%s  ReqArgs {...}:\n", prefix)
			for _, child := range n.ReqArgs {
				printAST(child, indent+2)
			}
		}

	case *GroupNode:
		fmt.Printf("%sGroupNode {...}:\n", prefix)
		for _, child := range n.Children {
			printAST(child, indent+1)
		}
	}
}

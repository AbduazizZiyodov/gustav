import argparse

from tools.ast_generator import main as ast_generator_main


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("operation")

    result = parser.parse_args()

    match result.operation:
        case "generate_ast":
            return ast_generator_main()
        case _:
            return -1


if __name__ == "__main__":
    raise SystemExit(main())

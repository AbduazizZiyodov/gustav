example:
	DEBUG=yes ./scripts/gus examples/file.gus

check:
	ruff format && mypy --strict --ignore-missing-imports --explicit-package-bases .

repl:
	DEBUG=yes ./scripts/gus

generate_ast:
	./scripts/generate_ast

test:
	pytest -s -vv --cov=gustav -cov-report=term-missing --cov-report=html testing

generate_diagram:
	ebnf2railroad -t Gustav-Grammar --write-style --no-text-formatting assets/.grammar
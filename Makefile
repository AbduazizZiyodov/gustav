example:
	./gus examples/file.gus

check:
	ruff format && mypy --strict --ignore-missing-imports --explicit-package-bases .

repl:
	./gus

generate_ast:
	python3 tools/generate_ast.py
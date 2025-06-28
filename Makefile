example:
	DEBUG=yes ./gus examples/file.gus

check:
	ruff format && mypy --strict --ignore-missing-imports --explicit-package-bases .

repl:
	./gus

generate_ast:
	python3 -m tools generate_ast
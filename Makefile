example:
	DEBUG=yes ./gus examples/file.gus

check:
	ruff format && mypy --strict --ignore-missing-imports --explicit-package-bases .

repl:
	DEBUG=yes ./gus
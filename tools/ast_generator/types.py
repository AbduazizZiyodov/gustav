import typing as t

__all__ = ("NodeDefinitions", "NodeDefinitionMapping")


class NodeDefinition(t.TypedDict):
    type: str
    name: str


NodeDefinitions = list[NodeDefinition]
NodeDefinitionMapping = dict[str, NodeDefinitions]

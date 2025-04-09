from .add_cube import register_add_cube_tool

def register_all_tools(mcp):
    """Register all refactored tools with the MCP server."""
    print("Registering PiccoloMCP refactored tools...")
    register_add_cube_tool(mcp)
    print("PiccoloMCP tool registration complete.")
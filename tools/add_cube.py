from mcp.server.fastmcp import FastMCP, Context
from typing import Dict, Any
from piccolo_connection import get_piccolo_connection

def register_add_cube_tool(mcp: FastMCP):
    """Register add cube tools with MCP server."""

    @mcp.tool()
    def add_cube(
        ctx: Context,
        name: str = "New Cube",
        position: Dict[str, float] = None,
        scale: Dict[str, float] = None,
    ) -> Dict[str, Any]:
        """Adds a Cube to the Piccolo scene.

        Args:
            name: The name of the Cube to add.
            position: The position of the Cube (x, y, z).
            scale: The scale of the Cube (x, y, z).

        Returns:
            Dictionary with operation results ('success', 'message', 'data').
        """
        try:
            # Prepare parameters, setting default values if none provided
            params = {
                "name": name,
                "position": position or {"x": 0, "y": 0, "z": 0},
                "scale": scale or {"x": 1, "y": 1, "z": 1},
            }

            # Send command to Unity to add the Cube
            response = get_unity_connection().send_command("add_cube", params)

            # Process Unity's response
            if response.get("success"):
                return {"success": True, "message": response.get("message", "Cube added successfully."), "data": response.get("data")}
            else:
                return {"success": False, "message": response.get("error", "An error occurred while adding the Cube.")}

        except Exception as e:
            return {"success": False, "message": f"Python error while adding Cube: {str(e)}"}

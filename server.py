from mcp.server.fastmcp import FastMCP, Context, Image
import logging
from dataclasses import dataclass
from contextlib import asynccontextmanager
from typing import AsyncIterator, Dict, Any, List
from config import config
from tools import register_all_tools
from piccolo_connection import get_piccolo_connection, PiccoloConnection

# Configure logging using settings from config
logging.basicConfig(
    level=getattr(logging, config.log_level),
    format=config.log_format
)
logger = logging.getLogger("PiccoloMCP")

# Global connection state
_piccolo_connection: PiccoloConnection = None

@asynccontextmanager
async def server_lifespan(server: FastMCP) -> AsyncIterator[Dict[str, Any]]:
    """Handle server startup and shutdown."""
    global _piccolo_connection
    logger.info("PiccoloMCP server starting up")
    try:
        _piccolo_connection = get_piccolo_connection()
        logger.info("Connected to Piccolo on startup")
    except Exception as e:
        logger.warning(f"Could not connect to Piccolo on startup: {str(e)}")
        _piccolo_connection = None
    try:
        # Yield the connection object so it can be attached to the context
        # The key 'bridge' matches how tools like read_console expect to access it (ctx.bridge)
        yield {"bridge": _piccolo_connection}
    finally:
        if _piccolo_connection:
            _piccolo_connection.disconnect()
            _piccolo_connection = None
        logger.info("PiccoloMCP server shut down")

# Initialize MCP server
mcp = FastMCP(
    "PiccoloMCP",
    description="Piccolo Editor integration via Model Context Protocol",
    lifespan=server_lifespan
)

# Register all tools
register_all_tools(mcp)

# Asset Creation Strategy

@mcp.prompt()
def asset_creation_strategy() -> str:
    """Guide for discovering and using Piccolo MCP tools effectively."""
    return (
        "Available Unity MCP Server Tools:\\n\\n"
        "- `manage_editor`: Controls editor state and queries info.\\n"
        "- `execute_menu_item`: Executes Unity Editor menu items by path.\\n"
        "- `read_console`: Reads or clears Unity console messages, with filtering options.\\n"
        "- `manage_scene`: Manages scenes.\\n"
        "- `manage_gameobject`: Manages GameObjects in the scene.\\n"
        "- `manage_script`: Manages C# script files.\\n"
        "- `manage_asset`: Manages prefabs and assets.\\n\\n"
        "Tips:\\n"
        "- Create prefabs for reusable GameObjects.\\n"
        "- Always include a camera and main light in your scenes.\\n"
    )

# Run the server
if __name__ == "__main__":
    mcp.run(transport='stdio')
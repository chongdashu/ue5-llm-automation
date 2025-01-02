import asyncio
import websockets
import json

async def handle_prompt(websocket):
    async for message in websocket:
        # Mock LLM response for testing
        response = {
            "type": "create_npc",
            "action": "follow_player",
            "properties": {
                "name": "BP_FollowerNPC",
                "follow_distance": 200
            }
        }
        await websocket.send(json.dumps(response))

async def main():
    async with websockets.serve(handle_prompt, "localhost", 8765):
        print("LLM server running on ws://localhost:8765")
        await asyncio.Future()  # run forever

if __name__ == "__main__":
    asyncio.run(main())
"""
# This will connect to the test_service and print out the messages it receives
python -m farm_ng.core.event_client --service-config config.json --service-name test_service
"""
from __future__ import annotations

from typing import Type
import logging
import grpc
from farm_ng.core import event_service_pb2_grpc
import asyncio
from farm_ng.core.stamp import get_monotonic_now, get_system_clock_now, StampSemantics
from farm_ng.core.uri_pb2 import Uri
from farm_ng.core.event_service_pb2 import (
    EventServiceConfig,
    SubscribeReply,
    SubscribeRequest,
    ListUrisRequest,
    ReqRepReply,
    ReqRepRequest,
)
from farm_ng.core.event_pb2 import Event
from farm_ng.core.timestamp_pb2 import Timestamp
from farm_ng.core.events_file_reader import payload_to_protobuf
from farm_ng.core.events_file_writer import make_proto_uri
import importlib
from google.protobuf.message import Message
from farm_ng.core.event_service import load_service_config, add_service_parser
import argparse
from google.protobuf.empty_pb2 import Empty

logging.basicConfig(level=logging.INFO)


class EventClient:
    """Generic client class to connect with the Amiga brain services.

    Internally implements an `asyncio` gRPC channel that is designed to be imported by service specific clients.
    """

    def __init__(self, config: EventServiceConfig) -> None:
        """Initializes the client.

        Args:
            config (EventServiceConfig): the grpc configuration data structure.

        Raises:
            ValueError: if the port or host are invalid.
        """
        if config.port == 0:
            raise ValueError(
                f"Invalid port: {config}, are you sure this is a client config?"
            )

        if config.host == "":
            raise ValueError(
                f"Invalid host: {config}, are you sure this is a client config?"
            )

        # the configuration data structure
        self._config: EventServiceConfig = config

        # the client logger
        self._logger: logging.Logger = logging.getLogger(f"{self.config.name}/client")

        # the number of messages sent
        self._counts: dict[str, int] = dict()

        # the gRPC channel and stub
        self.channel: grpc.aio.Channel | None = None
        self.stub: event_service_pb2_grpc.EventServiceStub | None = None

    @property
    def config(self) -> EventServiceConfig:
        """Returns the configuration data structure."""
        return self._config

    @property
    def logger(self) -> logging.Logger:
        """Returns the logger."""
        return self._logger

    @property
    def server_address(self) -> str:
        """Returns the composed address and port."""
        return f"{self.config.host}:{self.config.port}"

    async def _try_connect(self) -> bool:
        """Tries to connect to the server.

        Returns:
            bool: True if the connection was successful, False otherwise.
        """
        if self.stub is not None and await self.channel.channel_ready():
            self.logger.debug("Already connected to %s", self.server_address)
            return True

        try:
            # create an async connection with the server
            self.channel = grpc.aio.insecure_channel(self.server_address)
            await self.channel.channel_ready()
            self.stub = event_service_pb2_grpc.EventServiceStub(self.channel)

        except Exception as exc:
            self.logger.warning("Could not connect to %s: %s", self.server_address, exc)
            await asyncio.sleep(1)
            return False

        return True

    @property
    def server_address(self) -> str:
        """Returns the composed address and port."""
        return f"{self.config.host}:{self.config.port}"

    async def subscribe(self, request: SubscribeRequest, decode=True):
        response_stream = None
        while True:
            if not await self._try_connect():
                if response_stream is not None:
                    response_stream.cancel()
                    response_stream = None
                self.logger.warning(
                    f"{self.config} is not streaming or ready to stream"
                )
                continue

            if response_stream is None:
                # get the streaming object
                response_stream = self.stub.subscribe(request)

            try:
                # try/except so app doesn't crash on killed service
                response: SubscribeReply = await response_stream.read()
                response.event.timestamps.append(
                    get_monotonic_now(StampSemantics.DRIVER_RECEIVE)
                )
                assert response and response != grpc.aio.EOF, "End of stream"
            except Exception as exc:
                self.logger.warning("here %s", exc)
                response_stream.cancel()
                response_stream = None
                continue

            if decode and message_cls is None:
                # get the message class from the uri
                name: str
                package: str
                name, package = _parse_protobuf_descriptor(response.event.uri)
                message_cls = getattr(importlib.import_module(package), name)

            # decode the message from the payload
            if decode:
                message: Message = message_cls()
                message.ParseFromString(response.payload)
                yield response.event, message

            # return only the event and the payload
            yield response.event, response.payload

    async def listUris(self) -> list[Uri]:
        """Returns the list of uris.

        Returns:
            list[Uri]: the list of uris.
        """
        # try to connect to the server, if it fails return an empty list
        if not self.try_connect():
            self.logger.warning("Could not list uris: %s", self.server_address)
            return []

        try:
            response: ListUrisReply = await self.stub.listUris(ListUrisRequest())
            return response.uris
        except grpc.RpcError as err:
            self.logger.warning(
                "Could not list uris: %s\n err=%s", self.server_address, err
            )
            return []

    async def reqReq(
        self, path: str, message: Message, timestamps: list[Timestamp] | None = None
    ) -> ReqRepReply:
        if not await self._try_connect():
            self.logger.warning("Could not send: %s", self.server_address)
            return Event(), Empty()
        count = self._counts.get(path, 0)
        self._counts[path] = count + 1

        # add the timestamps before sending the message
        timestamps = timestamps or []
        timestamps.append(get_monotonic_now(semantics=StampSemantics.CLIENT_SEND))
        timestamps.append(get_system_clock_now(semantics=StampSemantics.CLIENT_SEND))

        # create the request, serialize the message and send it
        uri: Uri = make_proto_uri(path=path, message=message)
        uri.query += f"&service_name={self.config.name}"
        payload = message.SerializeToString()
        request = ReqRepRequest(
            event=Event(
                uri=uri,
                timestamps=timestamps,
                payload_length=len(payload),
                sequence=count,
            ),
            payload=payload,
        )
        reply: ReqRepReply = await self.stub.reqRep(request)
        reply.event.timestamps.append(
            get_monotonic_now(semantics=StampSemantics.CLIENT_SEND)
        )
        return reply


async def test_subscribe(client: EventClient, uri: Uri):
    async for event, message in client.subscribe(SubscribeRequest(uri=uri, every_n=1)):

        reply = await client.reqReq(event.uri.path + "/response", message)
        print(event.uri.path, event.sequence, message.value, reply.event.sequence)


async def test_smoke():
    parser = argparse.ArgumentParser()
    add_service_parser(parser)
    args = parser.parse_args()
    config_list, service_config = load_service_config(args)
    client = EventClient(service_config)
    async_subscriptions: list[asyncio.Task] = []

    uris = []
    while True:
        uris = await client.listUris()
        if len(uris) > 1:
            break
        await asyncio.sleep(1)

    for uri in uris:
        async_subscriptions.append(asyncio.create_task(test_subscribe(client, uri)))
    await asyncio.gather(*async_subscriptions)


if __name__ == "__main__":

    asyncio.run(test_smoke())

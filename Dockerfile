FROM python:3-slim AS stage

COPY server/* /app/
WORKDIR /app

RUN python3 -m venv /opt/venv
RUN /opt/venv/bin/pip install -r requirements.txt
RUN /opt/venv/bin/pip list -v

FROM python:3-slim

ARG VERSION_NUMBER

COPY --from=stage /app /app
COPY --from=stage /opt/venv /opt/venv
COPY .pio/build/lolin_d32/firmware.bin /assets/firmware-${VERSION_NUMBER}.bin
RUN echo "${VERSION_NUMBER}" > /.version

ENV PYTHONPATH /piplib

WORKDIR /app

ENTRYPOINT ["/opt/venv/bin/python"]
CMD ["main.py"]
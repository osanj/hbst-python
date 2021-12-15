FROM python:3.8.12-slim-buster

RUN pip install \
    pytest \
    numpy

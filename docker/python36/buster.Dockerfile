FROM python:3.6.15-slim-buster

RUN pip install \
    pytest \
    opencv-python-headless==4.*

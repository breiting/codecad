doc:
	podman run --rm -it -p 8000:8000 -v ${PWD}:/docs codecad-doc serve -f mkdocs.yml -a 0.0.0.0:8000

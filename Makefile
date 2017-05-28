

test:
	./httptest.py

deploy: www/d3.v3.min.js www/index.html
	cp www/d3.v3.min.js data
	cp www/index.html data
	cp passwd data

TAG = 1.0.0
PFX = iztokkucan/pykafarr_dev

build:
	docker build -t $(PFX):$(TAG) .

push: build
	docker push $(PFX):$(TAG)

run: 
	docker run --rm $(PFX):$(TAG)

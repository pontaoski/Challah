Product {
	name: "android_openssl"

	Export {
		Qt.android_support.extraLibs: [
			exportingProduct.sourceDirectory+"/android_openssl/latest/arm/libcrypto_1_1.so",
			exportingProduct.sourceDirectory+"/android_openssl/latest/arm/libssl_1_1.so",
			exportingProduct.sourceDirectory+"/android_openssl/latest/arm64/libcrypto_1_1.so",
			exportingProduct.sourceDirectory+"/android_openssl/latest/arm64/libssl_1_1.so",
			exportingProduct.sourceDirectory+"/android_openssl/latest/x86/libcrypto_1_1.so",
			exportingProduct.sourceDirectory+"/android_openssl/latest/x86/libssl_1_1.so",
			exportingProduct.sourceDirectory+"/android_openssl/latest/x86_64/libcrypto_1_1.so",
			exportingProduct.sourceDirectory+"/android_openssl/latest/x86_64/libssl_1_1.so",
		]
		Depends { name: "Qt.android_support" }
	}
}

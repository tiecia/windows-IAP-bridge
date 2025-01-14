#include "GetStoreProductsAsyncWorker.h"
#include <Windows.h>
#include <iostream>
#include <napi.h>
#include <string>
#include <winrt/Windows.Foundation.Collections.h>

GetStoreProductsAsyncWorker::GetStoreProductsAsyncWorker(const Napi::Function &callback, Napi::Array &productKinds,
                                                         WindowsStoreImpl *pImpl)
    : Napi::AsyncWorker(callback), m_productKinds(productKinds), m_pImpl(pImpl), m_result(NULL) {}

void GetStoreProductsAsyncWorker::Execute() { m_result = m_pImpl->GetStoreProducts(m_productKinds); }

void GetStoreProductsAsyncWorker::OnOK() {
  Napi::Env env = Env();
  Napi::Object obj = Napi::Object::New(env);
  while (m_result.HasCurrent()) {
    Napi::Object storeProd = Napi::Object::New(env);
    Napi::Object storePrice = Napi::Object::New(env);
    winrt::Windows::Foundation::Collections::IKeyValuePair<winrt::hstring,
                                                           winrt::Windows::Services::Store::StoreProduct>
        current = m_result.Current();

    storeProd.Set("inAppOfferToken", winrt::to_string(current.Value().InAppOfferToken()));

    auto price = current.Value().Price();
    storePrice.Set("formattedRecurrencePrice", winrt::to_string(price.FormattedRecurrencePrice()));
    storePrice.Set("formattedBasePrice", winrt::to_string(price.FormattedBasePrice()));
    storePrice.Set("formattedPrice", winrt::to_string(price.FormattedPrice()));
    storePrice.Set("currencyCode", winrt::to_string(price.CurrencyCode()));
    storeProd.Set("price", storePrice);
    storeProd.Set("storeId", winrt::to_string(current.Key()));

    obj.Set(winrt::to_string(current.Key()), storeProd);
    m_result.MoveNext();
  }
  Callback().MakeCallback(Receiver().Value(), {
                                                  env.Null(), // error first callback
                                                  obj         // value sent back to the callback
                                              });
}

void GetStoreProductsAsyncWorker::OnError(const Napi::Error &e) {
  Napi::Env env = Env();

  Callback().MakeCallback(Receiver().Value(), {e.Value(), env.Undefined()});
}

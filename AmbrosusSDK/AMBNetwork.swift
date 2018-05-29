//
//  Copyright: Ambrosus Technologies GmbH
//  Email: tech@ambrosus.com
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files 
// (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, 
// distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

import Foundation

/// The Network Configuration, modify this to set custom configuration
public final class AMBNetworkConfiguration: NSObject {

    /// The base path for the Ambrosus API
    public var ambrosusNetworkPath = "https://gateway-test.ambrosus.com/"

}

/// A Network Layer for interfacing with the Ambrosus API
public final class AMBNetwork: NSObject {

    static var configuration: AMBNetworkConfiguration = AMBNetworkConfiguration()

    fileprivate enum ResponseType {
        case json,
        data

        var accept: String? {
            return self == .json ? "application/json" : nil
        }
    }

    /// Request JSON data back from the API
    ///
    /// - Parameters:
    ///   - path: The path to the required endpoint, not including the basePath of "https://network.ambrosus.com/"
    ///   - completion: The data returned (optional, nil if request fails)
    private static func request(path: String, responseType: ResponseType = .json, completion: @escaping (_ data: Any?) -> Void) {
        guard let url = URL(string: path) else {
            completion(nil)
            print("URL Invalid")
            return
        }
        let request = URLRequest(url: url, responseType: responseType)
        let dataTask = URLSession.shared.dataTask(with: request) { (data, response, error) in
            guard let data = data, error == nil else {
                completion(nil)
                print(error?.localizedDescription ?? "")
                return
            }
            if responseType == .data {
                completion(data)
                return
            }
            do {
                let json = try JSONSerialization.jsonObject(with: data, options: .allowFragments)
                completion(json)
            } catch {
                completion(nil)
                print(error.localizedDescription)
            }
        }
        dataTask.resume()
    }

    /// Gets events back based on a scanner identifier
    ///
    /// - Parameters:
    ///   - query: The identifier including type of scanner e.g. [gtin]=0043345534
    ///   - completion: The events if available, nil if none returned
    public static func requestEvents(fromQuery query: String, completion: @escaping (_ data: [AMBEvent]?) -> Void) {
        let path = configuration.ambrosusNetworkPath + "events?data[type]=ambrosus.asset.identifier&data" + query
        request(path: path) { (data) in
            fetchEvents(from: data, completion: { (events) in
                guard let events = events else {
                    return
                }
                completion(events)
            })
        }
    }

    /// Gets a single asset back directly from an ID
    ///
    /// - Parameters:
    ///   - id: The identifier associated with the desired asset
    ///   - completion: The asset if available, nil if unavailable
    public static func requestAsset(fromId id: String, completion: @escaping (_ data: AMBAsset?) -> Void) {
        if let asset = AMBDataStore.sharedInstance.assetStore.fetch(withAssetId: id) {
            completion(asset)
            return
        }

        let path = configuration.ambrosusNetworkPath + "assets/" + id
        request(path: path) { (data) in
            guard let data = data as? [String: Any] else {
                NSLog("Error, no asset found for id:" + id)
                completion(nil)
                return
            }
            let asset = AMBAsset(json: data)
            completion(asset)
        }
    }

    private static func fetchEvents(from data: Any?, completion: @escaping (_ data: [AMBEvent]?) -> Void) {
        guard let data = data as? [String: Any],
            let results = data["results"] as? [[String: Any]] else {
                NSLog("Couldn't find events for data")
                completion(nil)
                return
        }
        let events = results.flatMap { AMBEvent(json: $0) }
        completion(events)
    }
    
    /// Fetches the events associated with an asset ID
    ///
    /// - Parameters:
    ///   - id: The identifier associated with the asset with desired events
    ///   - completion: The array of events if available, nil if unavailable
    public static func requestEvents(fromAssetId id: String, completion: @escaping (_ data: [AMBEvent]?) -> Void) {
        let path = configuration.ambrosusNetworkPath + "events?assetId=" + id
        request(path: path) { (data) in
            fetchEvents(from: data, completion: { (events) in
                guard let events = events else {
                    return
                }
                completion(events)
            })
        }
    }

    /// Downloads an image from a URL, if the image is already stored in the cache it will fetch it directly
    ///
    /// - Parameters:
    ///   - url: The URL of the image to download
    ///   - completion: The image if available, and optional error
    public static func requestImage(from url: URL, completion: @escaping (_ image: UIImage?, _ error: Error? ) -> Void) {
        if let cachedImage = AMBDataStore.sharedInstance.imageCache.object(forKey: url.absoluteString as NSString) {
            completion(cachedImage, nil)
        } else {
            request(path: url.absoluteString, responseType: .data, completion: { (data) in
                if let data = data as? Data, let image = UIImage(data: data) {
                    AMBDataStore.sharedInstance.imageCache.setObject(image, forKey: url.absoluteString as NSString)
                    completion(image, nil)
                } else {
                    completion(nil, nil)
                }
            })
        }
    }
}

fileprivate extension URLRequest {

    init(url: URL, responseType: AMBNetwork.ResponseType) {
        self = URLRequest(url: url)
        if let accept = responseType.accept {
            addValue(accept, forHTTPHeaderField: "Accept")
        }
    }

}
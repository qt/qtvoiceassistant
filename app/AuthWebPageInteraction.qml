/****************************************************************************
**
** Copyright (C) 2019 Luxoft Sweden AB
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Neptune 3 UI.
**
** $QT_BEGIN_LICENSE:GPL-QTAS$
** Commercial License Usage
** Licensees holding valid commercial Qt Automotive Suite licenses may use
** this file in accordance with the commercial license agreement provided
** with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and The Qt Company.  For
** licensing terms and conditions see https://www.qt.io/terms-conditions.
** For further information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
** SPDX-License-Identifier: GPL-3.0
**
****************************************************************************/

import QtQuick 2.0
import QtWebEngine 1.8

import alexaauth 1.0

QtObject {
    id: root

    property bool isAuthorizing: false
    property string authCode: ""
    property url authUrl: ""
    property int error: AlexaAuth.NoError
    property string httpUserAgent: "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/72.0.3626.119 Safari/537.36"
    property bool authorizationSucceed: false
    property string email: ""
    property string password: ""
    property bool  isFillingCaptcha: false
    property url captchaUrl: ""
    property string captcha: ""
    property WebEngineView authPage: WebEngineView {
        id: authPage

        property int retryCount: 0

        function callAuthAction() {
            switch (AlexaAuth.getAuthStage(authPage.title)){
            case AlexaAuth.AuthSignIn:
                    signinToAmazon();
                break;
            case AlexaAuth.AuthRegisterDevice:
                    registerDevice();
                break;
            case AlexaAuth.AuthError:
                    error = AlexaAuth.AutomaticAuthFailed;
                break;
            }
        }

        Component.onCompleted: {
            authPage.profile.httpAcceptLanguage = "en-US,en;q=0.9";
            httpUserAgent = authPage.profile.httpUserAgent;
        }
        onLoadingChanged: {
            if (loadRequest.status === WebEngineLoadRequest.LoadSucceededStatus) {
                callAuthAction()
            }
            if (loadRequest.status === WebEngineLoadRequest.LoadFailedStatus) {
                if (retryCount < 2) {
                    retryCount += 1
                    callAuthAction()
                } else {
                    error = AlexaAuth.AutomaticAuthFailed;
                }
            }
        }
    }

    function authorize()
    {
        if (AlexaAuth.parseJson()) {
            isAuthorizing = true;
            authPage.url = authUrl;
        } else {
            error = AlexaAuth.ConfigFileFailure;
        }
    }

    function authorizeWithCaptcha()
    {
        isFillingCaptcha = false
        inputEmail()
        inputPassword()
        inputCaptcha()
    }

    function signinToAmazon()
    {
        var js = AlexaAuth.getJSString(AlexaAuth.SignIn);
        authPage.runJavaScript(js, function(cb) {
                switch (AlexaAuth.signinToAmazonResult(cb)) {
                case AlexaAuth.SignInInputEmail:
                    inputEmail();
                    break
                case AlexaAuth.SignInCaptcha:
                    error = AlexaAuth.ImageRecognizionRequired;
                    showCaptcha()
                    break
                case AlexaAuth.SignInError:
                    error = AlexaAuth.ImageRecognizionRequired;
                    showCaptcha();
                    break
                }
            }
        )
    }

    function showCaptcha()
    {
        var js = AlexaAuth.getJSString(AlexaAuth.CaptchaSrc);
        authPage.runJavaScript(js, function(cb) {
            if (cb === null) {
                console.warn("Unable to get captcha")
            } else {
                root.isFillingCaptcha = true;
                root.captchaUrl = cb
            }
        } );
    }

    function inputCaptcha()
    {
        var js = AlexaAuth.getJSString(AlexaAuth.GetCaptchaInput);
        authPage.runJavaScript(js, function (cb) {
                if (cb === null) {
                    console.warn("Captcha field doesn't exist on the page.")
                    error = AlexaAuth.HtmlItemNotFound;
                } else {
                    var jsSet = AlexaAuth.getJSString(AlexaAuth.SetCaptcha, captcha);
                    authPage.runJavaScript(jsSet);
                    clickSignIn();
                }
            }
        );
    }

    function inputEmail()
    {
        var js = AlexaAuth.getJSString(AlexaAuth.GetEmailInput);
        authPage.runJavaScript(js, function (cb) {
                if (cb === null) {
                    console.warn("Email field doesn't exist on the page.")
                    error = AlexaAuth.HtmlItemNotFound;
                } else {
                    var jsSet = AlexaAuth.getJSString(AlexaAuth.SetEmail, email);
                    authPage.runJavaScript(jsSet);
                    inputPassword();
                }
            }
        );
    }

    function inputPassword()
    {
        var js = AlexaAuth.getJSString(AlexaAuth.GetPasswordInput);
        authPage.runJavaScript(js, function (cb) {
                if (cb === null) {
                    console.warn("Password field doesn't exist on the page")
                    error = AlexaAuth.HtmlItemNotFound;
                } else {
                    var jsSet = AlexaAuth.getJSString(AlexaAuth.SetPassword, password);
                    authPage.runJavaScript(jsSet);
                    clickSignIn();
               }
            }
        );
    }

    function clickSignIn()
    {
        var js = AlexaAuth.getJSString(AlexaAuth.GetClickSignIn);
        authPage.runJavaScript(js, function (cb) {
                if (cb === null) {
                    console.warn("Sign in button doesn't exist on the page")
                    error = AlexaAuth.HtmlItemNotFound;
                } else {
                    var jsSet = AlexaAuth.getJSString(AlexaAuth.ClickElement, js);
                    authPage.runJavaScript(jsSet);
               }
            }
        );
    }

    function registerDevice()
    {
        var js = AlexaAuth.getJSString(AlexaAuth.RegisterDeviceTitle);
        authPage.runJavaScript(js, function(cb) {
                switch (AlexaAuth.registerDeviceResult(cb)) {
                case AlexaAuth.RegisterDevice:
                    inputCode();
                    break
                case AlexaAuth.RegisterDeviceSuccess:
                    isAuthorizing = false;
                    authorizationSucceed = true;
                    break
                case AlexaAuth.RegisterDeviceError:
                    error = AlexaAuth.AutomaticAuthFailed;
                    break
                }
            }
        )
    }

    function inputCode()
    {
        var js = AlexaAuth.getJSString(AlexaAuth.GetInputCode);
        authPage.runJavaScript(js, function (cb) {
                if (cb === null) {
                    console.warn("No field for authorization code!")
                    error = AlexaAuth.HtmlItemNotFound;
                } else if (authCode.length > 0){
                    var jsSet = AlexaAuth.getJSString(AlexaAuth.SetInputCode, authCode);
                    authPage.runJavaScript(jsSet);
                    clickContinue();
               } else {
                    error = AlexaAuth.AutomaticAuthFailed;
               }
            }
        );
    }

    function clickContinue()
    {
        var js = AlexaAuth.getJSString(AlexaAuth.GetContinue);
        authPage.runJavaScript(js, function (cb) {
                if (cb === null) {
                    console.warn("Not found 'continue' button")
                    error = AlexaAuth.HtmlItemNotFound;
                } else {
                    var jsSet = AlexaAuth.getJSString(AlexaAuth.ClickElement, js);
                    authPage.runJavaScript(jsSet);
               }
            }
        );
    }
}

#pragma once

const String htmlText = F(R"rawliteral(
<!DOCTYPE html>
<html lang='en'>
<head>
    <meta charset='UTF-8'>
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>
    <title>Doorlock</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        body {
            display: flex;
            align-items: center;
            justify-content: center;
            min-height: 100vh;
            min-width: 100vw;
            background: rgb(33, 33, 33);
            color: rgb(235, 225, 225);
            font-family: 'Source Code Pro', 'Courier New', Courier, monospace;
        }

        main {
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
        }

        table {
            width: min(90vw, 40rem);
            margin: 1rem 0;
        }

        td, th {
            border: 0.15rem solid white;
            padding: 0.2rem 1rem;
        }

        th {
            padding: 1rem;
        }

        h3 {
            text-align: center;
        }

        button {
            width: 5rem;
            height: 2rem;
            background-color: #24ff5e;
            color: #212121;
            outline: none;
            border: none;
            border-radius: 0.5rem;
            font-weight: bold;
            font-size: 1rem;
            margin: 0.2rem;
            font-family: 'Source Code Pro', 'Courier New', Courier, monospace;
        }

        button:active {
            transform: scale(0.96);
        }
    </style>
</head>

<body>
    <main>
        <table>
            <tr>
                <th colspan='2'><h3>Fingerprint Sensor</h3></th>
            </tr>
            <tr>
                <td>Status</td>
                <td id='finger-status'>false</td>
            </tr>
            <tr>
                <td>Admin fingerprints</td>
                <td id='admin-count'>0</td>
            </tr>
            <tr>
                <td>Registered fingerprints</td>
                <td id='finger-count'>0</td>
            </tr>
            <tr>
                <td>Last fingerprint location</td>
                <td id='last-fingerprint'>0</td>
            </tr>
            <tr>
                <th colspan='2'><h3>IR Sensor</h3></th>
            </tr>
            <tr>
                <td>Last IR key</td>
                <td id='last-ir-key'>-</td>
            </tr>
            <tr>
                <th colspan='2'><h3>Door</h3></th>
            </tr>
            <tr>
                <td>Last opened</td>
                <td><span id='door-open-second'>0</span>s</td>
            </tr>
            <tr>
                <td>Reason</td>
                <td id='door-open-reason'>-</td>
            </tr>
            <tr>
                <td>Status</td>
                <td><h3><button id='door-status'>Closed</button></h3></td>
            </tr>
        </table>
    </main>
</body>

<script src='https://cdnjs.cloudflare.com/ajax/libs/crypto-js/4.2.0/crypto-js.min.js' integrity='sha512-a+SUDuwNzXDvz4XrIcXHuCf089/iJAoN4lmrXJg18XnduKK6YlDHNRalv4yd1N40OKI80tFidF+rqTFKGPoWFQ==' crossorigin='anonymous' referrerpolicy='no-referrer'></script>

<script>
    const doorOpenColor = '#ffe624';
    const doorClosedColor = '#24ff5e';

    const fieldIDs = [
        'finger-status',
        'admin-count',
        'finger-count',
        'last-fingerprint',
        'last-ir-key',
        'door-open-second',
        'door-open-reason',
        'door-status'
    ];

    async function updateData() {
        const response = await fetch('/status');
        const json = await response.json();
        fieldIDs.forEach((field) => {
            document.getElementById(field).textContent = json[field];
        });
        const doorButton = document.getElementById('door-status');
        if (doorButton.textContent === 'Closed') {
            doorButton.style.backgroundColor = doorClosedColor;
        } else {
            doorButton.style.backgroundColor = doorOpenColor;
        }
    }

    setInterval(() => {updateData().then()}, 1000);
</script>

<script>
    function getHash(message) {
        return CryptoJS.SHA1(message).toString();
    }
    
    document.getElementById('door-status').addEventListener('click', (event) => {
        event.preventDefault();
        fetch('/opendoor', {method: 'POST'}).then();
    });
</script>

</html>
)rawliteral");

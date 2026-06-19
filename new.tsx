import { createRoot } from 'react-dom/client';

export default function SwiqHome() {
    return (
        <div>
            <nav style={{
                backgroundColor: 'lightgrey',
                position: 'relative',
                top: 0,
                left: 0,
                height: '30px',
                padding: '10px',
                borderRadius: '999px',
                display: 'flex',
                flexDirection: 'column',
                alignItems: 'left',
                textAlign: 'left',
                fontFamily: 'arial'
            }}>
                <img src="Swiq.svg" width='25px' height='25px' style={{
                    borderRadius: '999px'
                }} />
            </nav>
        </div>
    )
}

const container = document.getElementById('root');
if (container) {
    const root = createRoot(container);
    root.render(<SwiqHome />);
}
